import numpy as np
from numba import jit
from collections import deque
import subprocess

def unpack(item):
    return np.unpackbits([[np.uint8(np.uint64(item) >> np.uint8(i))] for i in range(0, 64, 8)], axis=1)[:, ::-1]

def unpack_array(a):
    c = np.right_shift(a[:, np.newaxis], np.arange(0, 64, 8, dtype=np.uint8)).astype(np.uint8)
    return np.unpackbits(c[:, :, np.newaxis], axis=2)[:, :, ::-1]

ALL_PIECES = np.array([
    1,
    3,
    7,
    15,
    31,
    1 | (1<<8),
    3 | (3<<8),
    2 | (7<<8),
    1 | (7<<8),
    3 | (1<<8),
    1 | (1<<8) | (1<<16),
    7 | (4<<8) | (4<<16),
    7 | (7<<8) | (7<<16),
    1 | (1<<8) | (1<<16) | (1<<24),
    1 | (1<<8) | (1<<16) | (1<<24) | (1<<32),
    ], dtype=np.uint64)

class State(object):

    def __init__(self, board=None, pieces=None):
        if board is None:
            self.board = np.uint64(0)
        else:
            self.board = np.uint64(board)
        if pieces is None:
            self.pieces = np.zeros([3,], dtype=np.uint64)
        else:
            self.pieces = pieces

    def copy(self):
        state = State(self.board)
        state.pieces[:] = self.pieces
        return state

    def draw(self):
        if np.any(self.pieces):
            return
        self.pieces[:] = np.random.choice(ALL_PIECES, 3)

    @jit
    def explode(self):
        mask = np.uint64(0)
        h_line = np.uint64(255)
        v_line = np.uint64(72340172838076673)
        """
        v_line = np.uint64(1)
        for _ in range(8):
            v_line = v_line << np.uint8(8)
            v_line |= np.uint8(1)
        """
        if self.board & h_line == h_line:
            mask |= h_line
        if self.board & v_line == v_line:
            mask |= v_line
        for _ in range(7):
            h_line = h_line << np.uint8(8)
            v_line = v_line << np.uint8(1)
            if self.board & h_line == h_line:
                mask |= h_line
            if self.board & v_line == v_line:
                mask |= v_line
        self.board = self.board & (~mask)


class Simulator(object):

    def __init__(self):
        self.proc = subprocess.Popen("./a.out", stdin=subprocess.PIPE, stdout=subprocess.PIPE, encoding="utf-8")

    def stop(self):
        self.proc.communicate("EXIT\n")
        self.proc.terminate()
        self.proc = None

    def brute(self, state):
        self.proc.stdin.write("BRUTE %d %d %d %d\n" % (state.board, *state.pieces))
        self.proc.stdin.flush()
        num_moves = int(self.proc.stdout.readline())
        board_strs = [self.proc.stdout.readline() for _ in range(num_moves)]
        boards = np.array(board_strs, dtype=np.uint64)
        return boards

    def get_move(self, board):
        self.proc.stdin.write("GET_MOVE %d\n" % (board,))
        self.proc.stdin.flush()
        int_moves = int(self.proc.stdout.readline())
        if int_moves == -1:
            return False
        moves = []
        for _ in range(3):
            pos_x = int_moves & 7;
            int_moves = int_moves >> 3;
            pos_y = int_moves & 7;
            int_moves = int_moves >> 3;
            piece_index = int_moves & 3;
            int_moves = int_moves >> 2;
            moves.append((piece_index, pos_x, pos_y));
        return list(reversed(moves))
