#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if(eof) {
        eof_index = index + data.length();
        eof_flag = true;
    }
    
    if(index < cur_index && index + data.length() < cur_index) return;

    if(_unassembled_bytes.find(index) == _unassembled_bytes.end() || _unassembled_bytes[index].length() < data.length()) {
        _unassembled_bytes[index] = data;
    }

    check_overlap();
    write_assembled_bytes();

    if(eof_flag && cur_index == eof_index) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { 
    size_t cnt = 0;
    for(auto iter = _unassembled_bytes.begin(); iter != _unassembled_bytes.end(); ++iter) {
        if(iter->first <= cur_index) {
            continue;
        }
        else {
            cnt += iter->second.length();
        }
    }
    return cnt;
}

bool StreamReassembler::empty() const { return _unassembled_bytes.empty(); }

void StreamReassembler::check_overlap() {
    unordered_set<int> index2beDeleted;

    for(auto iter = _unassembled_bytes.begin(); iter != _unassembled_bytes.end(); ++iter) {
        if(index2beDeleted.find(iter->first) != index2beDeleted.end()) {
            continue;
        }

        if(iter->first < cur_index) {
            string left = iter->second.substr(cur_index - iter->first);
            if(_unassembled_bytes.find(cur_index) == _unassembled_bytes.end() || _unassembled_bytes[cur_index].length() < left.length()) {
                _unassembled_bytes[cur_index] = left;  
            } 
            index2beDeleted.emplace(iter->first);
        }
        else {
            size_t cur_max = iter->first + iter->second.length();
            auto iter_ol = iter;
            for(++iter_ol; iter_ol != _unassembled_bytes.end(); ++iter_ol) {
                if(iter_ol->first > cur_max) {
                    break;
                }
                else if(iter_ol->first + iter_ol->second.length() < cur_max) {
                    index2beDeleted.emplace(iter_ol->first);
                }
                else {
                    _unassembled_bytes[iter->first] += iter_ol->second.substr(cur_max - iter_ol->first);
                    cur_max = iter->first + iter->second.length();
                    index2beDeleted.emplace(iter_ol->first);
                }
            }
            iter = iter_ol;
            if(iter == _unassembled_bytes.end()) {
                break;
            }
        }
    }

    for(auto _index : index2beDeleted) {
        _unassembled_bytes.erase(_index);
    }
}

void StreamReassembler::write_assembled_bytes() {
    if(_unassembled_bytes.find(cur_index) == _unassembled_bytes.end()) {
        return;
    }

    size_t bytes_written = _output.write(_unassembled_bytes[cur_index]);
    string left = _unassembled_bytes[cur_index].substr(bytes_written);
    _unassembled_bytes.erase(cur_index);
    cur_index += bytes_written;
    if(left.length() > 0) {
        _unassembled_bytes[cur_index] = left;
    }
}