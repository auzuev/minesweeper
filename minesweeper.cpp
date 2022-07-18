#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Cell
{   
    bool is_open;
    int r; int c;

    int bombs_nr;

    //-- perimeter state
    int ass_bombs_around;
    int ass_undecided_around; // only unsaturated empties

    Cell() {
        is_open=false;
        bombs_nr = -1;
    }
    
};

struct Field // spatial features over Cell, invasively
{
    vector<vector<Cell> > xy___cell;
    int rows; int cols;
    int bombs_total;
    
    Field(int rows, int cols, int bombs) :rows(rows), cols(cols), bombs_total(bombs) {

    }

    void read() {
        empty_field(rows, cols);
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                char sym;
                cin >> sym;
                if (sym == '?') {
                    xy___cell[r][c].is_open = false;
                } else if (sym == '-') { 
                    xy___cell[r][c].is_open = true;
                    xy___cell[r][c].bombs_nr = 0;
                } else {
                    xy___cell[r][c].is_open = true;
                    xy___cell[r][c].bombs_nr = sym - '0';
                }
            }
        }
    };

    void empty_field(int rows, int cols) {
        this->rows = rows; this->cols = cols;
        xy___cell.resize(rows);
        for (int i=0;i<rows;i++) {
            xy___cell[i].resize(cols, Cell());
            for (int j=0;j<cols;j++) {
                xy___cell[i][j].r = i;
                xy___cell[i][j].c = j;
            }
        }
    }

    bool is_inside(int r, int c) {
        return (r >= 0) && (r < rows) && (c >= 0) && (c < cols);
    }

    bool has_empty_neighs(int r, int c) {
        for (int rd=-1;rd<2;rd++) {
            for (int cd=-1;cd<2;cd++) {
                if (!is_inside(r+rd, c+cd)) continue;
                if (!xy___cell[r + rd][c+cd].is_open) return true;
            }
        }
        return false;
    }

    bool has_open_neighs(Cell& cell){ 
        int r = cell.r; int c = cell.c;
        for (int rd=-1;rd<2;rd++) {
            for (int cd=-1;cd<2;cd++) {
                if (!is_inside(r+rd, c+cd)) continue;
                if (xy___cell[r + rd][c+cd].is_open) return true;
            }
        }
        return false;
    }

    // vector<Cell*> get_perimeter_cells() {
    //     vector<Cell*> cells;
    //     for (int i=0;i<rows;i++) {
    //         for (int j=0;j<cols;j++) {
    //             if (xy___cell[i][j].is_open && has_empty_neighs(i, j)) {
    //                 cells.push_back(&xy___cell[i][j]);
    //             }
    //         }
    //     }
    //     return cells;
    // }

    vector<Cell*> get_unknown_perimeter() {
        vector<Cell*> cells;
        for (auto& row: xy___cell) {
            for (Cell& cell: row) {
                if (cell.is_open) continue;
                if (has_open_neighs(cell)) cells.push_back(&cell);
            }
        }
        return cells;
    }

    vector<Cell*> get_unknown_detatched() {
        vector<Cell*> cells;
        for (auto& row: xy___cell) {
            for (Cell& cell: row) {
                if (cell.is_open) continue;
                if (!has_open_neighs(cell)) cells.push_back(&cell);
            }
        }
        return cells;
    }
};

struct Perimeter // Interactive part of Strategy model over frozen Field
{
    Field* field;
    vector <Cell* > unknowns;

    Perimeter (Field* field): field(field) {
        unknowns = field->get_unknown_perimeter();

        for (auto& row: field->xy___cell) {
            for (Cell& cell: row) {
                if (!cell.is_open) continue;
                cell.ass_bombs_around = 0;
                cell.ass_undecided_around = 0;
                int r = cell.r;
                int c = cell.c;
                for (int rd=-1;rd<2;rd++) {
                    for (int cd=-1;cd<2;cd++) {
                        if (!field->is_inside(r+rd, c+cd)) continue;
                        if (!field->xy___cell[r + rd][c+cd].is_open) {
                            cell.ass_undecided_around += 1;
                        }
                    }
                }
            }
        }
    }

    void makebomb(Cell* cell) {
        int r = cell->r; int c = cell->c;
        for (int rd=-1;rd<2;rd++) {
            for (int cd=-1;cd<2;cd++) {
                if (!field->is_inside(r+rd, c+cd)) continue;
                if (field->xy___cell[r + rd][c+cd].is_open) { 
                    field->xy___cell[r + rd][c+cd].ass_bombs_around += 1;
                    field->xy___cell[r + rd][c+cd].ass_undecided_around -= 1;
                }
            }
        }
    }

    void unmakebomb(Cell* cell) {
        int r = cell->r; int c = cell->c;
        for (int rd=-1;rd<2;rd++) {
            for (int cd=-1;cd<2;cd++) {
                if (!field->is_inside(r+rd, c+cd)) continue;
                if (field->xy___cell[r + rd][c+cd].is_open) {
                    field->xy___cell[r + rd][c+cd].ass_bombs_around -= 1;
                    field->xy___cell[r + rd][c+cd].ass_undecided_around += 1;
                }
            }
        }
    }

    void makeempty(Cell* cell) {
        int r = cell->r; int c = cell->c;
        for (int rd=-1;rd<2;rd++) {
            for (int cd=-1;cd<2;cd++) {
                if (!field->is_inside(r+rd, c+cd)) continue;
                if (field->xy___cell[r + rd][c+cd].is_open) { 
                    field->xy___cell[r + rd][c+cd].ass_undecided_around -= 1;
                }
            }
        }
    }

    void unmakeempty(Cell* cell) {
        int r = cell->r; int c = cell->c;
        for (int rd=-1;rd<2;rd++) {
            for (int cd=-1;cd<2;cd++) {
                if (!field->is_inside(r+rd, c+cd)) continue;
                if (field->xy___cell[r + rd][c+cd].is_open) { 
                    field->xy___cell[r + rd][c+cd].ass_undecided_around += 1;
                }
            }
        }
    }

    bool is_saturated(Cell* cell) {
        return cell->ass_bombs_around == cell->bombs_nr;
    }

    bool is_last_free_cell(Cell* cell) {
        return cell->ass_undecided_around == (cell->bombs_nr - cell->ass_bombs_around);
    }
    /* data */
    // perimeter open cell to unknown closed cells
};

struct Turn
{
    Cell* cell;
    bool action;
    // action 0 -- mark; 1 -- open
    Turn (){};
    Turn(Cell* c, bool a) {
        cell = c; action = a;
    }
};

struct Strategy 
{
    Field* field;
    vector<Cell*> unknown_perimeter;
    vector<Cell*> unknown_detatched;
    vector<Turn> turns;
    vector<vector<bool> > perimeter_assumptions;

    void init(Field& f) {
        field = &f;
        unknown_perimeter = field->get_unknown_perimeter();
        unknown_detatched = field->get_unknown_detatched();
        perimeter_assumptions.resize(unknown_perimeter.size()); // number of rows is number of cells in perimeter.
        turns.resize(0);

    }

    bool valid(vector<bool>& state, Perimeter& perim) {
        //! TODO
        // check field->bombs_nr, state sum and detatched size
        return true;

    }

    void generate_assumptions_recursive(Perimeter& perim, vector<bool>& state, int curr_idx) {
        if (curr_idx == state.size()) {
            if (valid(state, perim)) {
                for (int i = 0; i < state.size(); i++) {
                    perimeter_assumptions[i].push_back(state[i]); 
                }
                
            }
            return;
        }

        Cell* cell = perim.unknowns[curr_idx];
        // check neighbours
        bool bomb_ok = true; bool empty_ok = true;
        int r = cell->r; int c = cell->c;
        for (int rd=-1;rd<2;rd++) {
            for (int cd=-1;cd<2;cd++) {
                if (!field->is_inside(r+rd, c+cd)) continue;
                if (!field->xy___cell[r + rd][c+cd].is_open) continue;
                Cell *neigh = &(field->xy___cell[r + rd][c+cd]);

                if (perim.is_saturated(neigh)) bomb_ok = false;
                if (perim.is_last_free_cell(neigh)) empty_ok = false;
            }
        }

        // update state and get next cell
        if (bomb_ok) {
            state[curr_idx] = true;
            perim.makebomb(cell);
            generate_assumptions_recursive(perim, state, curr_idx + 1);
            perim.unmakebomb(cell);
        }
        if (empty_ok) {
            perim.makeempty(cell);
            state[curr_idx] = false;
            generate_assumptions_recursive(perim, state, curr_idx + 1);
            perim.unmakeempty(cell);
        }
        return;
    }

    void generate_assumptions() {
        // make a 2 parts graph unopen cell to neigh open cells
        Perimeter perimeter(field); 
        // recursively generate perm
        // push at every final state
        vector<bool> state(unknown_perimeter.size());
        generate_assumptions_recursive(perimeter, state, 0);
        // validate?

    }

    void solve() {       
        // fill perimeters
        generate_assumptions();
        cout << "*" << perimeter_assumptions[0].size() << endl;

        
        // cut out certain turns
        for (int perimeter_idx=0;perimeter_idx<unknown_perimeter.size();perimeter_idx++) {
            bool can_be_bomb = false;
            int ii = 0;
            for (auto x: perimeter_assumptions[perimeter_idx]) {
                ii++;
                can_be_bomb |= x;
                if (can_be_bomb) {
                    break;
                }
            }
            if (!can_be_bomb) {turns.push_back(Turn(unknown_perimeter[perimeter_idx], false));}
        }
        
    }
    
    vector<Turn> get_turns() {return turns;}
};

void print_turns(vector<Turn>& turns) {
    for (auto turn: turns) {
        cout << turn.cell->r << " " << turn.cell->c << endl;
    }
}


int main() {
    while(1) {
        Field f(16, 30, 99);
        f.read();
        Strategy s;
        s.init(f);
        s.solve();
        vector<Turn> turns = s.get_turns();
        print_turns(turns);
    }
}
