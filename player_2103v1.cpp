#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unordered_map>
#include <utility> // for pair
#include <climits>
#define max(a, b) (a > b) ? a : b
#define min(a, b) (a < b) ? a : b

int player;
const int SIZE = 8;

const int corner_value = 30;
const int side_value = 8;

struct Point
{
    int x, y;
    Point() : Point(0, 0) {}
    Point(float x, float y) : x(x), y(y) {}
    bool operator==(const Point &rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }
    bool operator!=(const Point &rhs) const
    {
        return !operator==(rhs);
    }
    Point operator+(const Point &rhs) const
    {
        return Point(x + rhs.x, y + rhs.y);
    }
    Point operator-(const Point &rhs) const
    {
        return Point(x - rhs.x, y - rhs.y);
    }
};

// special points in heuristic
const Point corner_point[4] =
    {
        Point(0, 0),
        Point(0, SIZE - 1),
        Point(SIZE - 1, 0),
        Point(SIZE - 1, SIZE - 1)};

// use the class provided with main.cpp
class OthelloBoard
{
public:
    enum SPOT_STATE
    {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{Point(-1, -1), Point(-1, 0), Point(-1, 1),
                                           Point(0, -1), /*{0, 0}, */ Point(0, 1),
                                           Point(1, -1), Point(1, 0), Point(1, 1)}};
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player;
    bool done;
    int winner;

private:
    int get_next_player(int player) const
    {
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const
    {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const
    {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc)
    {
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const
    {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const
    {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir : directions)
        {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY)
            {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center)
    {
        for (Point dir : directions)
        {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY)
            {
                if (is_disc_at(p, cur_player))
                {
                    for (Point s : discs)
                    {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }

public:
    OthelloBoard()
    {
        reset();
    }
    OthelloBoard(std::array<std::array<int, SIZE>, SIZE> input_board)
    {
        this->board = input_board;
        disc_count[WHITE] = 0;
        disc_count[BLACK] = 0;
        done = false;
        winner = -1;
        //for (int i = 0; i < SIZE; i++)
        //{
        //    for (int j = 0; j < SIZE; j++)
        //    {
        //        board[i][j] = EMPTY;
        //    }
        //}
    }
    void reset()
    {
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            {
                board[i][j] = EMPTY;
            }
        }
        board[3][4] = board[4][3] = BLACK;
        board[3][3] = board[4][4] = WHITE;
        cur_player = BLACK;
        disc_count[EMPTY] = 8 * 8 - 4;
        disc_count[BLACK] = 2;
        disc_count[WHITE] = 2;
        next_valid_spots = get_valid_spots();
        done = false;
        winner = -1;
    }
    std::vector<Point> get_valid_spots() const
    {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    bool put_disc(Point p)
    {
        if (!is_spot_valid(p))
        {
            // debug
            //std::cout << "invalid\n";
            winner = get_next_player(cur_player);
            done = true;
            return false;
        }
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        // Check Win
        if (next_valid_spots.size() == 0)
        {
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
            if (next_valid_spots.size() == 0)
            {
                // Game ends
                done = true;
                int white_discs = disc_count[WHITE];
                int black_discs = disc_count[BLACK];
                if (white_discs == black_discs)
                    winner = EMPTY;
                else if (black_discs > white_discs)
                    winner = BLACK;
                else
                    winner = WHITE;
            }
        }
        return true;
    }
    // heuristic calculation
    int get_heuristic()
    {
        int black = 0;
        int white = 0;
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            {
                Point p(i, j);
                if (board[i][j] == WHITE)
                {
                    // check corners
                    for (auto corner : corner_point)
                    {
                        if (p == corner)
                        {
                            white += corner_value;
                            break;
                        }
                    }
                    // check sides
                    if(i == 0 || i == SIZE-1 ||  j == 0 || j == SIZE-1)
                        white += side_value;
                    white++;
                }
                else if (board[i][j] == BLACK)
                {
                    // check corners
                    for (auto corner : corner_point)
                    {
                        if (p == corner)
                        {
                            black += corner_value;
                            break;
                        }
                    }
                    // check sides
                    if(i == 0 || i == SIZE-1 ||  j == 0 || j == SIZE-1)
                        black += side_value;
                    black++;
                }
            }
        }
        return black - white;
    }
    std::string encode_spot(int x, int y)
    {
        if (is_spot_valid(Point(x, y)))
            return ".";
        if (board[x][y] == BLACK)
            return "O";
        if (board[x][y] == WHITE)
            return "X";
        return " ";
    }
    std::string encode_output(bool fail = false)
    {
        std::stringstream ss;
        int i, j;
        ss << "+---------------+\n";
        for (i = 0; i < SIZE; i++)
        {
            ss << "|";
            for (j = 0; j < SIZE - 1; j++)
            {
                ss << encode_spot(i, j) << " ";
            }
            ss << encode_spot(i, j) << "|\n";
        }
        ss << "+---------------+\n";
        return ss.str();
    }
};

std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;
// use a unordered map to store calculated heuristic value and calculated valid steps
//std::unordered_map<OthelloBoard, std::vector<OthelloBoard>> visited;
void minimax_lddfs(std::ofstream &fout, OthelloBoard input);
int dls(OthelloBoard input, int limit, int alpha, int beta, bool max);

void read_board(std::ifstream &fin)
{
    fin >> player;
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            fin >> board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream &fin)
{
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++)
    {
        fin >> x >> y;
        next_valid_spots.push_back({x, y});
    }
}

void write_valid_spot(std::ofstream &fout)
{
    // Keep updating the output until getting killed.
    // use iddfs the search the tree
    OthelloBoard input(board);
    input.next_valid_spots = next_valid_spots;
    input.cur_player = player;
    minimax_lddfs(fout, input);
}

void minimax_lddfs(std::ofstream &fout, OthelloBoard input)
{
    // search the tree layer by layer
    // flush an output for each new layer
    bool mode;
    if (player == 1) // black
        mode = true;
    if (player == 2) // white
        mode = false;
    for (int i = 2; i < 64; i += 1)
    {
        std::pair<Point, int> results;
        int alpha = INT_MIN;
        int beta = INT_MAX;
        // find next boards and pair with a point
        std::vector<std::pair<OthelloBoard, Point>> next_board;
        std::vector<Point> next_move = input.next_valid_spots;
        for (auto i : next_move)
        {
            OthelloBoard copy = input;
            copy.put_disc(i);
            next_board.push_back(std::pair<OthelloBoard, Point>(copy, i));
        }
        // max
        if (mode)
        {
            results.second = INT_MIN;
            for (auto boards : next_board)
            {
                // place next move
                int ans = dls(boards.first, i, alpha, beta, false);
                if (ans > results.second)
                {
                    results.second = ans;
                    results.first = boards.second;
                }
                alpha = max(alpha, results.second);
                /*// debug
                std::cout << "max , depth = " << i + 1 << "\n"
                          << results.second << "\n"
                          << boards.first.encode_output();

                if (alpha >= beta) // alpha-beta pruning
                    break;*/
            }
        }
        // min
        else
        {
            results.second = INT_MAX;
            for (auto boards : next_board)
            {
                // place next move
                int ans = dls(boards.first, i, alpha, beta, true);
                if (ans < results.second)
                {
                    results.second = ans;
                    results.first = boards.second;
                }
                beta = min(beta, results.second);
                /*// debug
                std::cout << "min , depth = " << i + 1 << "\n"
                          << results.second << "\n"
                          << boards.first.encode_output();*/

                if (beta <= alpha)
                    break;
            }
        }
        // Remember to flush the output to ensure the last action is written to file.
        fout << results.first.x << " " << results.first.y << std::endl;
        fout.flush();
        // debug
        //std::cout << "out : " << results.first.x<< ", " << results.first.y << "\n";
    }
}

int dls(OthelloBoard input, int limit, int alpha, int beta, bool maxplayer)
{
    // debug
    //std::cout << "status : " << input.done << "\n";
    // if reach depth limit
    if (limit == 0)
        return input.get_heuristic();
    // if the game is over
    if (input.done)
    {
        return input.get_heuristic();
    }
    // find all next boards
    std::vector<OthelloBoard> next_board;
    //if (visited.find(input) != visited.cend()) // if calculated before
    //{
    //    next_board = visited[input];
    //}
    //else // find next boards
    //{
    std::vector<Point> next_move = input.get_valid_spots();
    next_move = input.get_valid_spots();
    for (auto i : next_move)
    {
        OthelloBoard copy = input;
        copy.put_disc(i);
        next_board.push_back(copy);
    }
    //visited[input] = next_board;
    //}
    // max
    if (maxplayer)
    {
        int value = INT_MIN;
        for (auto boards : next_board)
        {
            // place next move
            value = max(value, dls(boards, limit - 1, alpha, beta, false));
            alpha = max(alpha, value);
            // debug
            /*std::cout << "max , depth = " << limit << "\n"
                      << value << "\n"
                      << boards.encode_output();*/

            if (alpha >= beta) // alpha-beta pruning
                break;
        }
        return value;
    }
    // min
    else
    {
        int value = INT_MAX;
        for (auto boards : next_board)
        {
            // place next move
            value = min(value, dls(boards, limit - 1, alpha, beta, true));
            beta = min(beta, value);
            // debug
            /*std::cout << "min , depth = " << limit << "\n"
                      << value << "\n"
                      << boards.encode_output();*/

            if (beta <= alpha)
                break;
        }
        return value;
    }
}

int main(int, char **argv)
{
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}