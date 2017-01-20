#include "svrlib.h"
#include "svrutil.h"

using namespace svrutil;

int main(int argc, char* argv[]){
    puts("program start");
    EpollModule em;
    em.init();
    em.run();
    return 0;
}