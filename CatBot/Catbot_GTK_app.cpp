#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <raspicam/raspicam.h>

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

int fd;

GtkWidget *window;
GtkWidget *fixed;
GtkWidget *image;

raspicam::RaspiCam Camera;

int cam_on = 0;

void start_camera(GtkWidget *widget, gpointer data)
{
    //system("raspivid -p 50,100,400,300 -k&\n");
    cam_on = 1;
}

void stop_camera(GtkWidget *widget, gpointer data)
{
    //system("pkill raspivid");
    cam_on = -1;
}

void move_forward(GtkWidget *widget, gpointer data) // move forward
{
    write(fd, "1", 1);
}

void move_backwards(GtkWidget *widget, gpointer data) // move reverse
{
    write(fd, "2", 1);
}

void turn_left(GtkWidget *widget, gpointer data) // turn left
{
    write(fd, "3", 1);
}

void turn_right(GtkWidget *widget, gpointer data) // turn right
{
    write(fd, "4", 1);
}

void play_audio(GtkWidget *widget, gpointer data) // play audio
{
    system("play /home/pi/Music/CatTreats.wav");
}

void food_dispense(GtkWidget *widget, gpointer data) // dispense food
{
    write(fd, "6", 1);
}

void camera_up(GtkWidget *widget, gpointer data) // camera up
{
    write(fd, "7", 1);
}

void camera_down(GtkWidget *widget, gpointer data) // camera down
{
    write(fd, "8", 1);
}

void toy_on(GtkWidget *widget, gpointer data) // toy on
{
    write(fd, "9", 1);
}

void toy_off(GtkWidget *widget, gpointer data) // toy off
{
    write(fd, "A", 1);
}

gboolean key_handler(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    //printf(event->keyval);
    switch (event->keyval) {
        case GDK_KEY_w:
        case GDK_KEY_W:
            write(fd, "1", 1);
            break;
        case GDK_KEY_a:
        case GDK_KEY_A:
            write(fd, "3", 1);
            break;
        case GDK_KEY_d:
        case GDK_KEY_D:
            write(fd, "4", 1);
            break;
        case GDK_KEY_s:
        case GDK_KEY_S:
            write(fd, "2", 1);
            break;
    }
    return FALSE;
}

void reset(GtkWidget *widget, gpointer data) // reset
{
    write(fd, "0", 1);
}

gboolean image_thread(void *args) {
    if (cam_on == 1) {
        Camera.setCaptureSize(320, 240);
        Camera.grab();
        unsigned char *data=new unsigned char[  Camera.getImageTypeSize ( raspicam::RASPICAM_FORMAT_RGB )];
        Camera.retrieve ( data,raspicam::RASPICAM_FORMAT_RGB );//get camera image

        std::ofstream outFile ( "test.png",std::ios::binary );
        outFile<<"P6\n"<<Camera.getWidth() <<" "<<Camera.getHeight() <<" 255\n";
        outFile.write ( ( char* ) data, Camera.getImageTypeSize ( raspicam::RASPICAM_FORMAT_RGB ) );
        
        image = gtk_image_new_from_file("test.png");
        gtk_fixed_put(GTK_FIXED(fixed), image, 580, 10);
        gtk_widget_set_size_request(image, 320, 240);

        gtk_widget_queue_draw(image);
        gtk_widget_show_all(window);
        delete data;
    } else if (cam_on == -1) {
        image = gtk_image_new_from_file("blank.png");
        gtk_fixed_put(GTK_FIXED(fixed), image, 580, 10);
        gtk_widget_set_size_request(image, 320, 240);
        gtk_widget_queue_draw(image);
        gtk_widget_show_all(window);
        cam_on = 0;
    }
    return TRUE;
}

int cam_init() {
    //Open camera 
    std::cout<<"Opening Camera..."<<std::endl;

    Camera.setCaptureSize(320, 240);

    if ( !Camera.open()) {std::cerr<<"Error opening camera"<<std::endl;return -1;}
    //wait a while until camera stabilizes
    sleep(3);

    //Camera.grab();
    //unsigned char *data=new unsigned char[  Camera.getImageTypeSize ( raspicam::RASPICAM_FORMAT_RGB )];
    //Camera.retrieve ( data,raspicam::RASPICAM_FORMAT_RGB );

    //std::ofstream outFile ( "test.png",std::ios::binary );
    //outFile<<"P6\n"<<Camera.getWidth() <<" "<<Camera.getHeight() <<" 255\n";
    //outFile.write ( ( char* ) data, Camera.getImageTypeSize ( raspicam::RASPICAM_FORMAT_RGB ) );

    //delete data;
    return 0;
}

int main(int argc, char *argv[])
{
    //int fd;

    // Open the Port. We want read/write, no "controlling tty" status,
    // and open it no matter what state DCD is in
    fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NDELAY);
    //open mbed's USB virtual com port
    if (fd == -1)
    {
        perror("open_port: Unable to open /dev/ttyACM0 - ");
        //return(-1);
    }
    // Turn off blocking for reads, use (fd, F_SETFL, FNDELAY) if you want that
    fcntl(fd, F_SETFL, 0);
    //Linux Raw serial setup options
    struct termios options;
    tcgetattr(fd, &options);     //Get current serial settings in structure
    cfsetspeed(&options, B9600); //Change a only few
    options.c_cflag &= ~CSTOPB;
    options.c_cflag |= CLOCAL;
    options.c_cflag |= CREAD;
    cfmakeraw(&options);
    tcsetattr(fd, TCSANOW, &options); //Set serial to new settings
    sleep(1);

    if (cam_init() == -1) return -1;

    GtkWidget *btn_reset;     //reset
    GtkWidget *btn_forward;   //forward
    GtkWidget *btn_backward;  //back
    GtkWidget *btn_left;      //turn left
    GtkWidget *btn_right;     //turn right
    GtkWidget *btn_audio;     //play audio
    GtkWidget *btn_dispense;  //dispense food
    GtkWidget *btn_cam_up;    //camera up
    GtkWidget *btn_cam_down;  //camera down
    GtkWidget *btn_toy_on;    //toy on
    GtkWidget *btn_toy_off;   //toy off
    GtkWidget *btn_cam_start; //Start Camera
    GtkWidget *btn_cam_stop;  //Stop Camera

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "CatBot");
    gtk_window_set_default_size(GTK_WINDOW(window), 560, 260);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window), fixed);

    btn_reset = gtk_button_new_with_label("Reset");
    gtk_fixed_put(GTK_FIXED(fixed), btn_reset, 270, 90);
    gtk_widget_set_size_request(btn_reset, 80, 30);

    btn_forward = gtk_button_new_with_label("Forward(W)");
    gtk_fixed_put(GTK_FIXED(fixed), btn_forward, 260, 50);
    gtk_widget_set_size_request(btn_forward, 80, 30);

    btn_backward = gtk_button_new_with_label("Backward(S)");
    gtk_fixed_put(GTK_FIXED(fixed), btn_backward, 260, 130);
    gtk_widget_set_size_request(btn_backward, 80, 30);

    btn_left = gtk_button_new_with_label("Left(A)");
    gtk_fixed_put(GTK_FIXED(fixed), btn_left, 180, 90);
    gtk_widget_set_size_request(btn_left, 80, 30);

    btn_right = gtk_button_new_with_label("Right(D)");
    gtk_fixed_put(GTK_FIXED(fixed), btn_right, 360, 90);
    gtk_widget_set_size_request(btn_right, 80, 30);

    btn_audio = gtk_button_new_with_label("Play Audio");
    gtk_fixed_put(GTK_FIXED(fixed), btn_audio, 200, 170);
    gtk_widget_set_size_request(btn_audio, 80, 30);

    btn_dispense = gtk_button_new_with_label("Food");
    gtk_fixed_put(GTK_FIXED(fixed), btn_dispense, 320, 170);
    gtk_widget_set_size_request(btn_dispense, 80, 30);

    btn_cam_up = gtk_button_new_with_label("Camera Up");
    gtk_fixed_put(GTK_FIXED(fixed), btn_cam_up, 20, 20);
    gtk_widget_set_size_request(btn_cam_up, 80, 30);

    btn_cam_down = gtk_button_new_with_label("Camera Down");
    gtk_fixed_put(GTK_FIXED(fixed), btn_cam_down, 20, 60);
    gtk_widget_set_size_request(btn_cam_down, 80, 30);

    btn_toy_on = gtk_button_new_with_label("Toy On");
    gtk_fixed_put(GTK_FIXED(fixed), btn_toy_on, 460, 20);
    gtk_widget_set_size_request(btn_toy_on, 80, 30);

    btn_toy_off = gtk_button_new_with_label("Toy Off");
    gtk_fixed_put(GTK_FIXED(fixed), btn_toy_off, 460, 60);
    gtk_widget_set_size_request(btn_toy_off, 80, 30);

    btn_cam_start = gtk_button_new_with_label("Start Camera");
    gtk_fixed_put(GTK_FIXED(fixed), btn_cam_start, 20, 100);
    gtk_widget_set_size_request(btn_cam_start, 80, 30);

    btn_cam_stop = gtk_button_new_with_label("Stop Camera");
    gtk_fixed_put(GTK_FIXED(fixed), btn_cam_stop, 20, 140);
    gtk_widget_set_size_request(btn_cam_stop, 80, 30);

    image = gtk_image_new_from_file("blank.png");
    gtk_fixed_put(GTK_FIXED(fixed), image, 580, 10);
    gtk_widget_set_size_request(image, 320, 240);
    
    gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);

    // Connected buttons
    g_signal_connect(G_OBJECT(btn_cam_start), "clicked",
                     G_CALLBACK(start_camera), NULL);

    g_signal_connect(G_OBJECT(btn_cam_stop), "clicked",
                     G_CALLBACK(stop_camera), NULL);

    g_signal_connect(G_OBJECT(btn_forward), "pressed", // move forward
                     G_CALLBACK(move_forward), NULL);

    g_signal_connect(G_OBJECT(btn_backward), "pressed", // move backwards
                     G_CALLBACK(move_backwards), NULL);

    g_signal_connect(G_OBJECT(btn_left), "pressed", // turn left
                     G_CALLBACK(turn_left), NULL);

    g_signal_connect(G_OBJECT(btn_right), "pressed", // turn right
                     G_CALLBACK(turn_right), NULL);

    g_signal_connect(G_OBJECT(btn_audio), "clicked", // play audio
                     G_CALLBACK(play_audio), NULL);

    g_signal_connect(G_OBJECT(btn_dispense), "clicked", // dispense food
                     G_CALLBACK(food_dispense), NULL);

    g_signal_connect(G_OBJECT(btn_cam_up), "clicked", // camera up
                     G_CALLBACK(camera_up), NULL);

    g_signal_connect(G_OBJECT(btn_cam_down), "clicked", // camera down
                     G_CALLBACK(camera_down), NULL);

    g_signal_connect(G_OBJECT(btn_toy_on), "clicked", // toy on
                     G_CALLBACK(toy_on), NULL);

    g_signal_connect(G_OBJECT(btn_toy_off), "clicked", // toy off
                     G_CALLBACK(toy_off), NULL);

    g_signal_connect(G_OBJECT(btn_reset), "clicked", // reset
                     G_CALLBACK(reset), NULL);

    g_signal_connect(G_OBJECT(btn_forward), "released", // release reset
                     G_CALLBACK(reset), NULL);

    g_signal_connect(G_OBJECT(btn_backward), "released", // release reset
                     G_CALLBACK(reset), NULL);

    g_signal_connect(G_OBJECT(btn_left), "released", // release reset
                     G_CALLBACK(reset), NULL);

    g_signal_connect(G_OBJECT(btn_right), "released", // release reset
                     G_CALLBACK(reset), NULL);
                     
    g_signal_connect(G_OBJECT(window), "key_press_event", // release reset
                     G_CALLBACK(key_handler), NULL);
                     
    g_signal_connect(G_OBJECT(window), "key_release_event", // release reset
                     G_CALLBACK(reset), NULL);
        

    g_signal_connect(G_OBJECT(window), "destroy",
                     G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);

    gdk_threads_add_timeout(1000/5, image_thread, NULL);
    gdk_threads_init();
    gdk_threads_enter();

    gtk_main();
    gdk_threads_leave();

    // Don't forget to clean up and close the port
    tcdrain(fd);
    close(fd);

    return 0;
}
