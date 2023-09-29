#include "read.h"

using std::vector;
using std::cout;
using std::endl;

Read::Read()
{
    sequence_ = Sequence();
    name = "";
    number_of_threads_in_which_it_is_loaded = 0;
    size_ = 0;
}

Read::Read(std::string s, size_t size)
{
    sequence_ = Sequence(s);
    name = "";
    depth = -1;
    number_of_threads_in_which_it_is_loaded = 0;
    size_ = size;
}

void Read::upload_sequence(std::string s){
    sequence_ = Sequence(s);
    size_ = s.size();
    number_of_threads_in_which_it_is_loaded += 1;
}

void Read::free_sequence(){
    number_of_threads_in_which_it_is_loaded -= 1;
    if (number_of_threads_in_which_it_is_loaded == 0){
        sequence_ = Sequence();
    }
}

void Read::add_overlap(long int o){

    neighbors_.push_back(o);

}

size_t Read::size(){
    return size_;
}

void Read::new_backbone(std::pair<int, int> pair, size_t size){

    if (pair.second < size){
        backbone_seq.push_back(pair);
    }
    else{
        cout << pair.second << " " << size << endl;
        throw std::logic_error("Problem in backbone, too high neighbor index: ");
    }

}

void Read::add_link(size_t l, short end){
    if (end == 0){
        links_left.push_back(l);
    }
    else{
        links_right.push_back(l);
    }
}

std::vector<size_t> Read::get_links_left(){
    return links_left;
}

std::vector<size_t> Read::get_links_right(){
    return links_right;
}

void Read::set_position_in_file(long int p){
    positionInFile_ = p;
}

long int Read::get_position_in_file(){
    return positionInFile_;
}
