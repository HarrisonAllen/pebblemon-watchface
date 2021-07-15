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

def convert_locations_to_bin(in_file, out_file):
    open(out_file, 'wb').close()

    with open(in_file, 'r') as i_f:
        data = i_f.readline()
        while data:
            int_data = [int(i) for i in data.strip().split(',')]
            byte_data = bytes(int_data)
            print(f'{data}->{int_data}')
            with open(out_file, 'ab') as of:
                of.write(byte_data)
            data = i_f.readline()

if __name__ == "__main__":
    # reduce_path_size('assets/moves.txt', 'assets/path.bin')
    convert_locations_to_bin('assets/locations.txt', 'assets/Locations.bin')