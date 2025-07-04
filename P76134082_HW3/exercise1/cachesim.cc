#include "cachesim.h"
#include "common.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>

cache_sim_t::cache_sim_t(size_t _sets, size_t _ways, size_t _linesz, const char* _name)
: sets(_sets), ways(_ways), linesz(_linesz), name(_name), log(false)
{
  init();
  fifo_queues.resize(sets);
  for (size_t idx = 0; idx < sets; idx++) {
    for (size_t way = 0; way < ways; way++) {
      way_timestamps[idx * ways + way] = 0; // 初始時間戳為0
    }
  }
  next_timestamp.resize(sets, 1);
}

static void help()
{
  std::cerr << "Cache configurations must be of the form" << std::endl;
  std::cerr << "  sets:ways:blocksize" << std::endl;
  std::cerr << "where sets, ways, and blocksize are positive integers, with" << std::endl;
  std::cerr << "sets and blocksize both powers of two and blocksize at least 8." << std::endl;
  exit(1);
}

cache_sim_t* cache_sim_t::construct(const char* config, const char* name)
{
  const char* wp = strchr(config, ':');
  if (!wp++) help();
  const char* bp = strchr(wp, ':');
  if (!bp++) help();

  size_t sets = atoi(std::string(config, wp).c_str());
  size_t ways = atoi(std::string(wp, bp).c_str());
  size_t linesz = atoi(bp);

  if (ways > 4 /* empirical */ && sets == 1)
    return new fa_cache_sim_t(ways, linesz, name);
  return new cache_sim_t(sets, ways, linesz, name);
}

void cache_sim_t::init()
{
  if (sets == 0 || (sets & (sets-1)))
    help();
  if (linesz < 8 || (linesz & (linesz-1)))
    help();

  idx_shift = 0;
  for (size_t x = linesz; x>1; x >>= 1)
    idx_shift++;

  tags = new uint64_t[sets*ways]();
  way_timestamps = new uint64_t[sets*ways]();  // 新增：時間戳記數組
  read_accesses = 0;
  read_misses = 0;
  bytes_read = 0;
  write_accesses = 0;
  write_misses = 0;
  bytes_written = 0;
  writebacks = 0;

  miss_handler = NULL;
}

cache_sim_t::cache_sim_t(const cache_sim_t& rhs)
 : sets(rhs.sets), ways(rhs.ways), linesz(rhs.linesz),
   idx_shift(rhs.idx_shift), name(rhs.name), log(false)
{
  tags = new uint64_t[sets*ways];
  way_timestamps = new uint64_t[sets*ways]; // 新增：時間戳記數組
  memcpy(tags, rhs.tags, sets*ways*sizeof(uint64_t));
  memcpy(way_timestamps, rhs.way_timestamps, sets*ways*sizeof(uint64_t)); // 複製時間戳記
  
  // 複製FIFO佇列和下一個時間戳
  fifo_queues = rhs.fifo_queues;
  next_timestamp = rhs.next_timestamp;
}

cache_sim_t::~cache_sim_t()
{
  print_stats();
  delete [] tags;
  delete [] way_timestamps; // 新增：釋放時間戳記數組
}

void cache_sim_t::print_stats()
{
  float mr = 100.0f*(read_misses+write_misses)/(read_accesses+write_accesses);

  std::cout << std::setprecision(3) << std::fixed;
  std::cout << name << " ";
  std::cout << "Bytes Read:            " << bytes_read << std::endl;
  std::cout << name << " ";
  std::cout << "Bytes Written:         " << bytes_written << std::endl;
  std::cout << name << " ";
  std::cout << "Read Accesses:         " << read_accesses << std::endl;
  std::cout << name << " ";
  std::cout << "Write Accesses:        " << write_accesses << std::endl;
  std::cout << name << " ";
  std::cout << "Read Misses:           " << read_misses << std::endl;
  std::cout << name << " ";
  std::cout << "Write Misses:          " << write_misses << std::endl;
  std::cout << name << " ";
  std::cout << "Writebacks:            " << writebacks << std::endl;
  std::cout << name << " ";
  std::cout << "Miss Rate:             " << mr << '%' << std::endl;
}

uint64_t* cache_sim_t::check_tag(uint64_t addr)
{
  size_t idx = (addr >> idx_shift) & (sets-1);
  size_t tag = (addr >> idx_shift) | VALID;

  for (size_t i = 0; i < ways; i++)
    if (tag == (tags[idx*ways + i] & ~DIRTY))
      return &tags[idx*ways + i];

  return NULL;
}

uint64_t cache_sim_t::victimize(uint64_t addr)
{
  size_t idx = (addr >> idx_shift) & (sets-1);
  size_t way;
  uint64_t victim;
  
  // 檢查是否有無效的way可以使用
  bool found_invalid = false;
  for (way = 0; way < ways; way++) {
    if ((tags[idx*ways + way] & VALID) == 0) {
      found_invalid = true;
      break;
    }
  }
  
  if (found_invalid) {
    // 如果有無效的way，直接使用它
    victim = tags[idx*ways + way];
  } else {
    // 使用FIFO策略：找出最早進入的way (時間戳最小的)
    uint64_t oldest_timestamp = UINT64_MAX;
    for (size_t i = 0; i < ways; i++) {
      if (way_timestamps[idx*ways + i] < oldest_timestamp) {
        oldest_timestamp = way_timestamps[idx*ways + i];
        way = i;
      }
    }
    victim = tags[idx*ways + way];
  }
  
  // 設置新的tag和時間戳
  tags[idx*ways + way] = (addr >> idx_shift) | VALID;
  way_timestamps[idx*ways + way] = next_timestamp[idx]++;
  
  return victim;
}

void cache_sim_t::access(uint64_t addr, size_t bytes, bool store)
{
  store ? write_accesses++ : read_accesses++;
  (store ? bytes_written : bytes_read) += bytes;

  uint64_t* hit_way = check_tag(addr);
  if (likely(hit_way != NULL))
  {
    if (store)
      *hit_way |= DIRTY;
    return;
  }

  store ? write_misses++ : read_misses++;
  if (log)
  {
    std::cerr << name << " "
              << (store ? "write" : "read") << " miss 0x"
              << std::hex << addr << std::endl;
  }

  uint64_t victim = victimize(addr);

  if ((victim & (VALID | DIRTY)) == (VALID | DIRTY))
  {
    uint64_t dirty_addr = (victim & ~(VALID | DIRTY)) << idx_shift;
    if (miss_handler)
      miss_handler->access(dirty_addr, linesz, true);
    writebacks++;
  }

  if (miss_handler)
    miss_handler->access(addr & ~(linesz-1), linesz, false);

  if (store)
    *check_tag(addr) |= DIRTY;
}

void cache_sim_t::clean_invalidate(uint64_t addr, size_t bytes, bool clean, bool inval)
{
  uint64_t start_addr = addr & ~(linesz-1);
  uint64_t end_addr = (addr + bytes + linesz-1) & ~(linesz-1);
  uint64_t cur_addr = start_addr;
  while (cur_addr < end_addr) {
    uint64_t* hit_way = check_tag(cur_addr);
    if (likely(hit_way != NULL))
    {
      if (clean) {
        if (*hit_way & DIRTY) {
          writebacks++;
          *hit_way &= ~DIRTY;
        }
      }

      if (inval)
        *hit_way &= ~VALID;
    }
    cur_addr += linesz;
  }
  if (miss_handler)
    miss_handler->clean_invalidate(addr, bytes, clean, inval);
}

fa_cache_sim_t::fa_cache_sim_t(size_t ways, size_t linesz, const char* name)
  : cache_sim_t(1, ways, linesz, name)
{
  // 初始化全相聯快取的FIFO佇列
  fifo_entry_order.clear();
}

uint64_t* fa_cache_sim_t::check_tag(uint64_t addr)
{
  auto it = tags.find(addr >> idx_shift);
  return it == tags.end() ? NULL : &it->second;
}

uint64_t fa_cache_sim_t::victimize(uint64_t addr)
{
  uint64_t tag = addr >> idx_shift;
  uint64_t old_tag = 0;
  
  if (tags.size() == ways) {
    // 快取已滿，使用FIFO策略替換最早進入的tag
    uint64_t victim_tag = fifo_entry_order.front();
    fifo_entry_order.pop_front(); // 從佇列前端移除最早的tag
    
    auto it = tags.find(victim_tag);
    if (it != tags.end()) {
      old_tag = it->second;
      tags.erase(it);
    }
  }
  
  // 將新tag添加到快取中並將其放入佇列末尾
  tags[tag] = tag | VALID;
  fifo_entry_order.push_back(tag); // 添加到佇列末尾
  
  return old_tag;
}
