import os
import glob

move_to_num = {
    'u': 0,
    'l': 1,
    'd': 2,
    'r': 3
}

def reduce_path_size(filename, out_filename):
    open(out_filename, 'wb').close()

    with open(filename, 'r') as f:
        all_data = f.readlines()
        while all_data:
            data = [move_to_num[all_data.pop(0).strip()] for _ in range(4)]
            b = 0
            for d in range(len(data)):
                b |= int(data[d]) << (2 * d)
            print(f'{data}->{b:>08b}')
            with open(out_filename, 'ab') as of:
                of.write(bytes([b]))

if __name__ == "__main__":
    reduce_path_size('assets/moves.txt', 'assets/path.bin')