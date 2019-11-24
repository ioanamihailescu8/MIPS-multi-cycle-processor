/*************************************************************************************|
|   1. YOU ARE NOT ALLOWED TO SHARE/PUBLISH YOUR CODE (e.g., post on piazza or online)|
|   2. Fill main.c and memory_hierarchy.c files                                       |
|   3. Do not use any other .c files neither alter main.h or parser.h                 |
|   4. Do not include any other library files                                         |
|*************************************************************************************/
#include "mipssim.h"

/// @students: declare cache-related structures and variables here
struct cache_data{
    int valid;
    int tag;
    int data_block[16];
};

struct cache_data *cache;
int size_index, num_blocks, number_of_tag_bits;

int make_word(int a0,int a1, int a2, int a3){
    return a0 << 24 | a1 << 16 | a2 << 8 | a3;
}

void memory_state_init(struct architectural_state* arch_state_ptr) {
    arch_state_ptr->memory = (uint32_t *) malloc(sizeof(uint32_t) * MEMORY_WORD_NUM);
    memset(arch_state_ptr->memory, 0, sizeof(uint32_t) * MEMORY_WORD_NUM);
    if(cache_size == 0){
        // CACHE DISABLED
        memory_stats_init(arch_state_ptr, 0); // WARNING: we initialize for no cache 0
    
    }else {
        // CACHE ENABLED
        
        num_blocks = cache_size / 16;
        cache = malloc(num_blocks * sizeof(struct cache_data));
        size_index = ceil(log2(num_blocks));
        number_of_tag_bits = 32 - size_index - 4;
        //printf("number of tag bits is: %d", number_of_tag_bits);
        memory_stats_init(arch_state_ptr, number_of_tag_bits);

    }
}


// returns data on memory[address / 4]
int memory_read(int address){
    arch_state.mem_stats.lw_total++;
    printf("\n address: %d \n", address);
    check_address_is_word_aligned(address);
    
    if(cache_size == 0){
        // CACHE DISABLED
        return (int) arch_state.memory[address / 4];
    }else{
        // CACHE ENABLED
        
        int offset = get_piece_of_a_word(address, 0, 4);
        int index = get_piece_of_a_word(address, 4, size_index);
        int tag  = get_piece_of_a_word(address, size_index+4, number_of_tag_bits);
       
        //printf("offset: %d, tag: %d, index %d \n", offset,tag,index);
        if (cache[index].tag == tag && cache[index].valid == 1) {
            arch_state.mem_stats.lw_cache_hits++;
            return make_word(cache[index].data_block[offset], cache[index].data_block[offset+1], cache[index].data_block[offset+2], cache[index].data_block[offset+3]);
        }
        else{
            cache[index].tag = tag;
            cache[index].valid = 1;
            for (int i = 0; i<4; i++)
                for (int j=0; j<4; j++){
                    cache[index].data_block[i*4+0] = get_piece_of_a_word(arch_state.memory[(address-offset+i*4+j) / 4], 24, 8);
                    cache[index].data_block[i*4+1] = get_piece_of_a_word(arch_state.memory[(address-offset+i*4+j) / 4], 16, 8);
                    cache[index].data_block[i*4+2] = get_piece_of_a_word(arch_state.memory[(address-offset+i*4+j) / 4], 8, 8);
                    cache[index].data_block[i*4+3] = get_piece_of_a_word(arch_state.memory[(address-offset+i*4+j) / 4], 0, 8);
                }
            
            return (int) arch_state.memory[address / 4];
        }

    }
    return 0;
}

// writes data on memory[address / 4]
void memory_write(int address, int write_data){
    arch_state.mem_stats.sw_total++;
    check_address_is_word_aligned(address);
    //printf("\n \n address is: %d \n \n", address);
    if(cache_size == 0){
        // CACHE DISABLED
        arch_state.memory[address / 4] = (uint32_t) write_data;
    }else{
        // CACHE ENABLED
        /// @students: your implementation must properly increment: arch_state_ptr->mem_stats.sw_cache_hits
        int offset = get_piece_of_a_word(address, 0, 4);
        int tag  = get_piece_of_a_word(address, size_index+4, number_of_tag_bits);
        int index = get_piece_of_a_word (address, 4, size_index);

        printf("offset: %d, tag: %d, index %d \n", offset,tag,index);

        if (cache[index].tag == tag && cache[index].valid == 1){

            arch_state.memory[address / 4] = (uint32_t) write_data;
            arch_state.mem_stats.sw_total++;

            cache[index].data_block[offset+0] = get_piece_of_a_word(write_data, 24, 8);
            cache[index].data_block[offset+1] = get_piece_of_a_word(write_data, 16, 8);
            cache[index].data_block[offset+2] = get_piece_of_a_word(write_data, 8, 8);
            cache[index].data_block[offset+3] = get_piece_of_a_word(write_data, 0, 8);
        }
        else
            arch_state.memory[address / 4] = (uint32_t) write_data;

    }
}
