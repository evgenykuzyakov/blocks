#include <cstdio>
#include <string>
#include <iostream>
#include <cstring>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <deque>
#include <unordered_set>

using namespace std;

constexpr const char* COMMAND_BRUTE = "BRUTE";
constexpr const char* COMMAND_GET_MOVE = "GET_MOVE";
constexpr const char* COMMAND_EXPLODE = "EXPLODE";
constexpr const char* COMMAND_EXIT = "EXIT";


typedef unsigned long long uint64;
typedef unsigned char uint8;


struct State {
  uint64 board;
  int pieces[3];

  State(): board(0) {
    memset(pieces, 0, sizeof(pieces));
  }

  State(const uint64 _board, const int _pieces[3]): board(_board) {
    memcpy(pieces, _pieces, sizeof(pieces));
  }

  State(const State& b): State(b.board, b.pieces) { }

  State copy() const {
    return State(*this);
  }

  static uint64 explode(uint64 board) {
    uint64 mask = 0;
    uint64 h_line = 255ull;
    uint64 v_line = 72340172838076673ull;
    for (int i = 0; i < 8; ++i) {
      if ((board & h_line) == h_line) {
        mask |= h_line;
      }
      if ((board & v_line) == v_line) {
        mask |= v_line;
      }
      h_line <<= 8;
      v_line <<= 1;
    }
    return board ^ mask;
  }

  void explode() {
    board = explode(board);
  }

  int pieces_mask() const {
    return (pieces[0] ? 1 : 0) | (pieces[1] ? 2 : 0) | (pieces[2] ? 4 : 0);
  }
};


struct Piece {
  uint64 mask;
  int index, width, height;
  Piece(): mask(0), index(0), width(0), height(0) { }
  Piece(uint64 _mask, int _index): mask(_mask), index(_index), width(0), height(0) {
    for (int i = 0; i < 8; ++i) {
      for (int j = 0; j < 8; ++j) {
        if ((mask >> (i * 8 + j)) & 1) {
          height = max(height, i + 1);
          width = max(width, j + 1);
        }
      }
    }
    // cout << "PIECE " << mask << " " << width << " " << height << endl;
  }
};

unordered_map<uint64, int> pieces_index;
vector<Piece> pieces;
unordered_set<uint64> visited_states[8];
vector<uint64 > brute_results;
unordered_map<uint64, int> brute_moves;
Piece brute_state_pieces[3];

int index_piece(uint64 mask) {
  const auto& it = pieces_index.find(mask);
  if (it != pieces_index.end()) {
    return it->second;
  }
  const int index = (int)pieces.size();
  pieces.emplace_back(mask, index);
  pieces_index.emplace(mask, index);
  return index;
}

bool can_place(const State& state, int piece_index, int pos_x, int pos_y) {
  const Piece& piece = pieces[state.pieces[piece_index]];
  // Check the piece is not used
  if (piece.index == 0) {
    return false;
  }
  // Check the piece fits in the board size
  if (pos_x + piece.width > 8 || pos_y + piece.height > 8) {
    return false;
  }
  // Check the piece fits in empty places
  if (state.board & (piece.mask << (pos_y * 8 + pos_x))) {
    return false;
  }
  return true;
}

State place(const State& state, int piece_index, int pos_x, int pos_y) {
  const Piece& piece = pieces[state.pieces[piece_index]];
  const uint64 new_board = state.board | (piece.mask << (pos_y * 8 + pos_x));
  State new_state(new_board, state.pieces);
  new_state.pieces[piece_index] = 0;
  return new_state;
}

int add_move(int previous_moves, int piece_index, int pos_y, int pos_x) {
  return (previous_moves << 8) | (piece_index << 6) | (pos_y << 3) | pos_x;
}

void brute(uint64 board, int pieces_mask, int previous_moves) {
  if (visited_states[pieces_mask].find(board) != visited_states[pieces_mask].end()) {
    return;
  }
  visited_states[pieces_mask].emplace(board);
  if (pieces_mask == 0) {
    brute_results.emplace_back(board);
    brute_moves[board] = previous_moves;
    return;
  }
  for (int i = 0; i < 3; ++ i) {
    if ((pieces_mask >> i) & 1) {
      const Piece& piece = brute_state_pieces[i];
      for (int j = 0; j < 9 - piece.height; ++j) {
        for (int k = 0; k < 9 - piece.width; ++k) {
          const uint64 mask = piece.mask << (j * 8 + k);
          if (!(board & mask)) {
            brute(
                State::explode(board | mask),
                pieces_mask ^ (1 << i),
                add_move(previous_moves, i, j, k));
          }
        }
      }
    }
  }
}

void run_brute() {
  State state;
  cin >> state.board;
  for (int i = 0; i < 3; ++i) {
    uint64 piece;
    cin >> piece;
    state.pieces[i] = index_piece(piece);
    brute_state_pieces[i] = pieces[state.pieces[i]];
  }
  state.explode();
  for (int i = 0; i < 8; ++i) {
    visited_states[i].clear();
  }
  brute_results.clear();
  brute_moves.clear();
  brute(state.board, state.pieces_mask(), 0);
  printf("%d\n", (int)brute_results.size());
  for (uint64 board : brute_results) {
    printf("%llu\n", board);
  }
}

void run_explode() {
  State state;
  cin >> state.board;
  state.explode();
  cout << state.board << endl;
}

void run_get_move() {
  uint64 board;
  cin >> board;
  const auto& it = brute_moves.find(board);
  if (it != brute_moves.end()) {
    cout << it->second << endl;
  } else {
    cout << -1 << endl;
  }
}

int main() {
  index_piece(0);
  while (1) {
    string command;
    cin >> command;
    if (command == COMMAND_BRUTE) {
      run_brute();
    } else if (command == COMMAND_EXPLODE) {
      run_explode();
    } else if (command == COMMAND_GET_MOVE) {
      run_get_move();
    } else if (command == COMMAND_EXIT) {
      return 0;
    } else {
      cout << "Uknown command " << command << endl;
    }
  }
  return -1;
}
