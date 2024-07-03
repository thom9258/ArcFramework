#include <arc/window.hpp>

//https://gist.github.com/koute/7391344
//https://learnopengl.com/Getting-started/Hello-Window

#define WIDTH 1200
#define HEIGHT 900

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    arc::Window window = arc::Window::create("mywindow", 0, 0, 1200, 800, 0);
    sleep(2);
}
