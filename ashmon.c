/**
 * ashmon version 1.3
 * ashmon a free and open source tool for bandwidth monitoring in linux with gui .
 * (C) 2015 by Mohammad Nejati www.github.com/ashtum
 * Released under the GPL v2.0
 *
 *  compile command : gcc ashmon.c -o ashmon -lX11 -pthread
 *  	you need install libx11-dev for compile u can get it with :
 *  		sudo apt-get install libx11-dev
 * 		or :
 *  		sudo yum install libX11-devel
 */
 
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/timeb.h>
#include <sys/stat.h>

typedef struct
{
    unsigned int last_rx;
    unsigned int last_tx;
    unsigned int tx_rates[100];
    unsigned int rx_rates[100];
    char * rx_rate_str;
    char * tx_rate_str;
    char * rx_str;
    char * tx_str;
    char * dev_name;
    int max_rate;
   
} linker;

linker mainlink;


int drawable = 0;


void * link_speed();
char * readfile();
char * readable_fs(u_int8_t,double);
char * file_to_string();
void timerFired();
void change_opacity();
void win_fade_out();
void win_fade_in();


int main(int argc, char *argv[]) {

    mainlink.dev_name = argv[1];
    if(!argv[1]) {
        mainlink.dev_name = (char *)"eth0";
    }
   
   
    Display *dis;
    Window win;
    GC gc;
	XSetWindowAttributes attr;
	XVisualInfo vinfo;
	
    dis = XOpenDisplay(0);
    
    if (!XMatchVisualInfo(dis, DefaultScreen(dis),  32, TrueColor, &vinfo)){
		XMatchVisualInfo(dis, DefaultScreen(dis),  24, TrueColor, &vinfo);
	}
   
   
    attr.colormap = XCreateColormap(dis, DefaultRootWindow(dis), vinfo.visual, AllocNone);
    attr.border_pixel = 0;
    attr.background_pixel = 0;  
	win = XCreateWindow(dis, DefaultRootWindow(dis), 0, 0, 176, 145, 0, vinfo.depth, InputOutput, vinfo.visual, CWColormap | CWBorderPixel |CWBackPixel, &attr);
    gc = XCreateGC(dis, win, 0, 0);
    XStoreName(dis, win, (char *)"Ashtum Monitor");
    XSelectInput(dis, win,ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask  | StructureNotifyMask );
    
   
    
   
    Atom property;
    property =  XInternAtom(dis, "_NET_WM_STATE_SKIP_TASKBAR", False);
    XChangeProperty(dis, win, XInternAtom(dis, "_NET_WM_STATE", False), XA_ATOM, 32, PropModeReplace, (unsigned char *) &property, 1);
    property =  XInternAtom(dis, "_NET_WM_STATE_ABOVE", False);
    XChangeProperty(dis, win, XInternAtom(dis, "_NET_WM_STATE", False), XA_ATOM, 32, PropModeAppend, (unsigned char *) &property,1);
    property =  XInternAtom(dis, "_NET_WM_WINDOW_TYPE_DOCK", False);
    XChangeProperty(dis, win, XInternAtom(dis, "_NET_WM_WINDOW_TYPE", False), XA_ATOM, 32, PropModeReplace, (unsigned char *) &property,1);
   
    #define MWM_HINTS_FUNCTIONS     (1L << 0)
    #define MWM_HINTS_DECORATIONS   (1L << 1)
    #define MWM_FUNC_MOVE           (1L << 2)
	#define MWM_DECOR_BORDER        (1L << 1)
	typedef struct {
		long    flags;
		long    functions;
		long    decorations;
		long    input_mode;
		long    status; } MotifWmHints;
     
    MotifWmHints   mwm_hints;
    property = XInternAtom(dis, "_MOTIF_WM_HINTS", False);
    mwm_hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    mwm_hints.functions=  MWM_FUNC_MOVE;
    mwm_hints.decorations = MWM_DECOR_BORDER ;
    XChangeProperty(dis, win, property, property, 32, PropModeReplace, (unsigned char *)&mwm_hints, 5);
   
   
    XEvent ev;
    Window window_returned;
    XWindowAttributes xwa;
   
    int grab = 0;
    int mowed = 0;
    int showed = 0;
    int lastm_x;
    int lastm_y;
    int lastw_x;
    int lastw_y;
    int x11_fd = ConnectionNumber(dis);
    fd_set in_fds;
    FILE *fptr;
    struct timeval tv;
    struct timeb timer;
    struct timeb timecache; 
    int root_x, root_y;
    int win_x = 350, win_y = 350;
    int opacity = 60;
    unsigned int mask_return;
    char home_patch[256];
    strcpy(home_patch, getenv ("HOME"));
    strcat(home_patch,"/.ashmon_config");
    
    void save_config_on_file(){
		fptr = fopen(home_patch, "wb+");
		if(fptr!= NULL){
			XTranslateCoordinates( dis, win, DefaultRootWindow(dis), 0, 0, &win_x, &win_y, &window_returned );
			XGetWindowAttributes( dis, win, &xwa );
			
			fprintf(fptr,"%i\n%i\n%i", opacity , win_x - xwa.x , win_y - xwa.y);
			fclose(fptr);
		}
	} 
    
	fptr = fopen(home_patch, "r");
	if(fptr)
	{
		// read config variables
		const size_t line_size = 6;
		char* line = malloc(line_size);
		int i = 0;
		while (fgets(line, line_size, fptr) != NULL){
			if(i ==0){
				opacity = atoi(line);
			}else if(i==1){
				win_x = atoi(line);
			}else if(i==2){
				win_y = atoi(line);
			}
			i++;
		}
		fclose(fptr);
	}else{
		fptr = fopen(home_patch, "wb+");
		if(fptr){
			chmod(home_patch,0666);
			save_config_on_file();
			fclose(fptr);
		}
	}
	
	// set config variables
	change_opacity(dis, &win,&opacity);
	XMoveWindow(dis, win , win_x , win_y);
	
    
    XMapWindow(dis, win);
    XFlush(dis);
    
    pthread_t pth;
    pthread_create(&pth,NULL,link_speed,0);
    
    
    
    while(1) {
       
        FD_ZERO(&in_fds);
        FD_SET(x11_fd, &in_fds);
        tv.tv_usec = 0;
        tv.tv_sec = 1;
       
        while(XPending(dis)){
           
            ftime(&timecache);
            if((1000.0 * timecache.time + timecache.millitm) - (1000.0 * timer.time + timer.millitm) > 1000){
                timerFired(dis, &win , &gc);
                ftime(&timer);
            }
           
            XNextEvent(dis, &ev);
            
            if(ev.type == 4) {
                if (ev.xbutton.button==Button1){
                    grab = 1;
                    mowed = 0;
                    XQueryPointer(dis, DefaultRootWindow(dis), &window_returned,&window_returned, &lastm_x, &lastm_y, &win_x, &win_y,&mask_return);
                    XTranslateCoordinates( dis, win, DefaultRootWindow(dis), 0, 0, &win_x, &win_y, &window_returned );
                    XGetWindowAttributes( dis, win, &xwa );
                    lastw_x = win_x - xwa.x;
                    lastw_y = win_y - xwa.y;
                }else if (ev.xbutton.button == Button4){
                    if(opacity<=95){
                        opacity += 5;
                        change_opacity(dis, &win,&opacity);
                        save_config_on_file();
                    }
                }else if (ev.xbutton.button == Button5){
                    if(opacity>15){
                        opacity -= 5;
                        change_opacity(dis, &win,&opacity);
                        save_config_on_file();
                    }
                }else if (ev.xbutton.button == Button3){
                    exit(0);
                }
            }
            if(ev.type == 5) {
                if (ev.xbutton.button==1){
                    grab = 0;
                    if(!mowed) {
                        win_fade_out(dis, &win,&opacity);
                        XUnmapWindow(dis, win);
                        showed = 5;
                    }else{
						save_config_on_file();
					}
                }
            }
            if(ev.type == 6 && grab == 1) {
                XQueryPointer(dis, DefaultRootWindow(dis), &window_returned,&window_returned, &root_x, &root_y, &win_x, &win_y,&mask_return);
                XMoveWindow(dis, win , lastw_x + (root_x-lastm_x) , lastw_y + (root_y-lastm_y));
                mowed = 1;
            }
        }
           
        // Wait for X Event or a Timer
       
        if (!select(x11_fd+1, &in_fds, 0, 0, &tv)){
            if (showed == 3) {
                XMapWindow(dis, win);
                XMoveWindow(dis, win, lastw_x, lastw_y);
                showed --;
            } else if(showed ==1) {
                win_fade_in(dis, &win,&opacity);
                showed = 0;
            } else if(showed > 0) {
                showed --;
            }
            ftime(&timer);
            timerFired(dis, &win , &gc);
        }
       
    }
    return(0);
}


void * link_speed(){
    while(1){
        usleep(990000);
        drawable = 0;
        usleep(10000);
        char * source;
        int i;
        char * devs;
        asprintf(&devs,"%s%s%s", "/sys/class/net/", mainlink.dev_name,"/statistics/rx_bytes");
        source =file_to_string(devs);
       
        for (i = 99; i !=0; i--) {
            mainlink.rx_rates[i] = mainlink.rx_rates[i-1];
        }
        if(mainlink.last_rx > 0) {
            mainlink.rx_rates[0] = atoi(source) - mainlink.last_rx ;
        } else {
            mainlink.rx_rates[0] = 0;
        }
        mainlink.rx_rate_str = readable_fs(1,mainlink.rx_rates[0]);
        mainlink.last_rx = atoi(source);
		mainlink.rx_str = readable_fs(0,mainlink.last_rx);


        asprintf(&devs,"%s%s%s","/sys/class/net/", mainlink.dev_name,"/statistics/tx_bytes");
        source =file_to_string(devs);
        for (i = 99; i !=0; i--) {
            mainlink.tx_rates[i] = mainlink.tx_rates[i-1];
        }

        if(mainlink.last_tx > 0) {
            mainlink.tx_rates[0] = atoi(source) - mainlink.last_tx ;
        } else {
            mainlink.tx_rates[0] = 0;
        }
        mainlink.tx_rate_str = readable_fs(1,mainlink.tx_rates[0]);
        mainlink.last_tx = atoi(source);
        mainlink.tx_str = readable_fs(0,mainlink.last_tx);
        
        mainlink.max_rate = 0;
        for(i=0; i<100; i++)
        {
            if( mainlink.rx_rates[i] >mainlink.max_rate ) {
                mainlink.max_rate=mainlink.rx_rates[i];
            }
            if( mainlink.tx_rates[i] >mainlink.max_rate ) {
                mainlink.max_rate=mainlink.tx_rates[i];
            }
        }
        drawable = 1;
    }
    return NULL;
}


void timerFired(Display * dis,Window  *win, GC * gc) {
    if(drawable){
        int i;
        unsigned int line_h;
        
        int upload_color = 0xFFBD2D00;
        int download_color = 0xFF008344;
        int upload_download_color = 0xFFFFB300;
        int background_color = 0xBBBBBBBB;
        int primary_color = 0xFF119999;
        
		// remove shapes
        XSetForeground(dis, *gc, background_color);
        XFillRectangle(dis, *win, *gc, 0, 0, 176, 145);
       
       
        
        // primry shape draws
        XSetForeground(dis, *gc, primary_color);
        XDrawLine(dis, *win, *gc, 5, 106, 170, 106);   
		XDrawRectangle(dis, *win, *gc, 0, 0, 175, 144);
        for (i=0; i<5; i++) {
            char * label_str ;
            label_str = readable_fs(1,(6.00-i)*(mainlink.max_rate/6.00));

            XDrawString(dis, *win, *gc, 5, i * 100/5 +15, label_str, strlen(label_str));
        }

		
        XDrawString(dis, *win, *gc, 92, 140, "UL:", 3);
        XDrawString(dis, *win, *gc, 113, 140, mainlink.tx_str, strlen(mainlink.tx_str));
        XDrawString(dis, *win, *gc, 5, 140, "DL:", 3);
        XDrawString(dis, *win, *gc, 25, 140, mainlink.rx_str, strlen(mainlink.rx_str));

        // red shape draws
        XSetForeground(dis, *gc, upload_color);
        XDrawString(dis, *win, *gc, 92, 123, "U:", 2);
        XDrawString(dis, *win, *gc, 106, 123, mainlink.tx_rate_str, strlen(mainlink.tx_rate_str));
        for(i = 0; i<99; i++) {
            if (mainlink.tx_rates[i] > 0) {
                line_h = ((mainlink.tx_rates[i])/(mainlink.max_rate+0.00)) *100.0 ;
            } else {
                line_h = 0;
            }

            if (line_h>0 && mainlink.tx_rates[i] >= mainlink.rx_rates[i]) {
                XDrawLine(dis, *win, *gc, 70+(100-i), 105, 70+(100-i), 5+(100-line_h));
            }
        }

        // green shape draws
        XSetForeground(dis, *gc, download_color);
        XDrawString(dis, *win, *gc, 5, 123, "D:", 2);
        XDrawString(dis, *win, *gc, 18, 123, mainlink.rx_rate_str, strlen(mainlink.rx_rate_str));
        for(i = 0; i<99; i++) {
            if (mainlink.rx_rates[i] > 0) {
                line_h = ((mainlink.rx_rates[i])/(mainlink.max_rate+0.00)) *100.0 ;
            } else {
                line_h = 0;
            }
            if (line_h>0 && mainlink.tx_rates[i] < mainlink.rx_rates[i]) {
                XDrawLine(dis, *win, *gc, 70+(100-i), 105, 70+(100-i), 5+(100-line_h));
            }
        }

        // yelow shape draws
        XSetForeground(dis, *gc, upload_download_color);
        for(i = 0; i<99; i++) {

            if(mainlink.tx_rates[i] > mainlink.rx_rates[i]) {
                line_h = (mainlink.rx_rates[i]/(mainlink.max_rate+0.00)) *100.0 ;
            } else {
                line_h = (mainlink.tx_rates[i]/(mainlink.max_rate+0.00)) *100.0 ;
            }

            if (line_h>0) {
                XDrawLine(dis, *win, *gc, 70+(100-i), 105, 70+(100-i), 5+(100-line_h));
            }
        }
    }   
}


char * file_to_string(char * address){
    char * buffer = "";
    long length;
    FILE * f = fopen (address, "rb");

    if (f)
    {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = malloc (length);
        if (buffer)
        {
            fread (buffer, 1, length, f);
        }
        fclose (f);
    }
   
    return buffer;
}

char * readable_fs(u_int8_t pers,double size) {
    char * buf=(char *) malloc(sizeof(char) * 10);
    int i = 0;
    char* units[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    while (size > 1024) {
        size /= 1024;
        i++;
    }

	if(pers){
		sprintf(buf, "%.*f %s/s", i, size, units[i]);
	}else{
		sprintf(buf, "%.*f %s", i, size, units[i]);
	}
    
    return buf;
}

void change_opacity(Display * dis,Window  *win, unsigned int * opacity){
     unsigned int real_opacity = 0xffffffff;
     real_opacity = ((*opacity / 100.0) * 0xffffffff);
     XChangeProperty( dis, *win, XInternAtom( dis, "_NET_WM_WINDOW_OPACITY", False) ,XA_CARDINAL, 32, PropModeReplace,(unsigned char*)&real_opacity,1L);
}

void win_fade_out(Display * dis,Window  *win, unsigned int * opacity){
    unsigned int reduser = *opacity;
    while(reduser>0){
        change_opacity(dis, win,&reduser);
        reduser-=1;
        usleep(3000);
        XFlush(dis);
    }
}

void win_fade_in(Display * dis,Window  *win, unsigned int * opacity){
    unsigned int reduser = 0;
    while(reduser<*opacity){
        change_opacity(dis, win,&reduser);
        reduser+=1;
        usleep(3000);
        XFlush(dis);
    }
}
