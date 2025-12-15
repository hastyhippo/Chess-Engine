#include "../misc/defines.h"
#include "../representation/board.h"
#include "../movegen/movegen.h"
#include "../search/search.h"

using namespace std;

class UCI {
    private:
        bool debug = false;
        Board board;
        bool stop_search = false;
        
        // Helper functions
        Move parse_uci_move(string uci_move);
        string move_to_uci(Move m);
        
    public:
        UCI();
        void loop();
        void uci();
        void isready();
        void set_option(string cmd);
        void register_engine(string cmd);
        void uci_new_game();
        void position(string cmd);
        void go(string cmd);
        void stop();
        void ponderhit();
        void debug_cmd(string cmd);
};