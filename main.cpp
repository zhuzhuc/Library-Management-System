#include "Library.h"
#include "src/cli/LibraryCliController.h"

int main() {
    Library library("小Z图书馆", "C++面向对象课程设计-zzc");
    cli::LibraryCliController controller(library);
    controller.bootstrap();
    controller.run();
    return 0;
}
