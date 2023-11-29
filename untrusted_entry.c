#include <untrusted_util.h>
#include <api_untrusted.h>

//extern uintptr_t region1;
extern uintptr_t region2;
extern uintptr_t region3;

extern uintptr_t enclave_start;
extern uintptr_t enclave_end;

#define SHARED_MEM_SYNC (0x90000000)

#define STATE_0 1
#define STATE_1 2
#define STATE_2 3
#define STATE_3 4

#define EVBASE 0x20000000

void untrusted_entry(int core_id, uintptr_t fdt_addr) {
  volatile int *flag = (int *) SHARED_MEM_SYNC;
  console_init();

  if(core_id != 0) {
    printm("Core n %d\n\n", core_id);
    while(true) {
      if(*flag == STATE_1) {
       api_result_t res = sm_region_update();
       if(res == MONITOR_OK) {
        *flag = STATE_2;
       }
      }
    };
  }

  *flag = STATE_0;
  
  //uint64_t region1_id = addr_to_region_id((uintptr_t) &region1);
  uint64_t region2_id = addr_to_region_id((uintptr_t) &region2);
  uint64_t region3_id = addr_to_region_id((uintptr_t) &region3);

  api_result_t result;

  printm("\n");

  printm("Region block\n");

  result = sm_region_block(region3_id);
  if(result != MONITOR_OK) {
    printm("sm_region_block FAILED with error code %d\n\n", result);
    test_completed();
  }
  
  printm("Region block\n");

  result = sm_region_block(region2_id);
  if(result != MONITOR_OK) {
    printm("sm_region_block FAILED with error code %d\n\n", result);
    test_completed();
  }
    
  *flag = STATE_1;
  while(*flag != STATE_2);

  printm("Region free\n");

  result = sm_region_free(region2_id);
  if(result != MONITOR_OK) {
    printm("sm_region_free FAILED with error code %d\n\n", result);
    test_completed();
  }
  
  printm("Region free\n");

  result = sm_region_free(region3_id);
  if(result != MONITOR_OK) {
    printm("sm_region_free FAILED with error code %d\n\n", result);
    test_completed();
  }

  printm("Region Metadata Create\n");

  result = sm_region_metadata_create(region3_id);
  if(result != MONITOR_OK) {
    printm("sm_region_metadata_create FAILED with error code %d\n\n", result);
    test_completed();
  }

  uint64_t region_metadata_start = sm_region_metadata_start();

  enclave_id_t enclave_id = ((uintptr_t) &region3) + (PAGE_SIZE * region_metadata_start);
  uint64_t num_mailboxes = 1;

  printm("Enclave Create\n");

  result = sm_enclave_create(enclave_id, EVBASE, REGION_MASK, num_mailboxes, true);
  if(result != MONITOR_OK) {
    printm("sm_enclave_create FAILED with error code %d\n\n", result);
    test_completed();
  }

  printm("Region assign\n");

  result = sm_region_assign(region2_id, enclave_id);
  if(result != MONITOR_OK) {
    printm("sm_region_assign FAILED with error code %d\n\n", result);
    test_completed();
  }

  uintptr_t enclave_handler_address = (uintptr_t) &region2;
  uintptr_t enclave_handler_stack_pointer = enclave_handler_address + HANDLER_LEN + (STACK_SIZE * NUM_CORES);

  printm("Enclave Load Handler\n");

  result = sm_enclave_load_handler(enclave_id, enclave_handler_address);
  if(result != MONITOR_OK) {
    printm("sm_enclave_load_handler FAILED with error code %d\n\n", result);
    test_completed();
  }

  uintptr_t page_table_address = enclave_handler_stack_pointer;

  printm("Enclave Load Page Table\n");

  result = sm_enclave_load_page_table(enclave_id, page_table_address, EVBASE, 3, NODE_ACL);
  if(result != MONITOR_OK) {
    printm("sm_enclave_load_page_table FAILED with error code %d\n\n", result);
    test_completed();
  }

  page_table_address += PAGE_SIZE;

  printm("Enclave Load Page Table\n");

  result = sm_enclave_load_page_table(enclave_id, page_table_address, EVBASE, 2, NODE_ACL);
  if(result != MONITOR_OK) {
    printm("sm_enclave_load_page_table FAILED with error code %d\n\n", result);
    test_completed();
  }

  page_table_address += PAGE_SIZE;

  printm("Enclave Load Page Table\n");

  result = sm_enclave_load_page_table(enclave_id, page_table_address, EVBASE, 1, NODE_ACL);
  if(result != MONITOR_OK) {
    printm("sm_enclave_load_page_table FAILED with error code %d\n\n", result);
    test_completed();
  }

  uintptr_t phys_addr = page_table_address + PAGE_SIZE;
  uintptr_t untrusted_addr = (uintptr_t) &enclave_start;
  uintptr_t virtual_addr = EVBASE;

  uint64_t size = ((uint64_t) &enclave_end) - ((uint64_t) &enclave_start);
  int num_pages_enclave = size / PAGE_SIZE;

  if((size % PAGE_SIZE) != 0) num_pages_enclave++;

  for(int i = 0; i < num_pages_enclave; i++) {

    printm("Enclave Load Page\n");

    result = sm_enclave_load_page(enclave_id, phys_addr, virtual_addr, untrusted_addr, LEAF_ACL);
    if(result != MONITOR_OK) {
      printm("sm_enclave_load_page FAILED with error code %d\n\n", result);
      test_completed();
    }

    phys_addr    += PAGE_SIZE;
    untrusted_addr      += PAGE_SIZE;
    virtual_addr += PAGE_SIZE;

  }

  //uintptr_t enclave_sp = virtual_addr;

  uint64_t size_enclave_metadata = sm_enclave_metadata_pages(num_mailboxes);

  thread_id_t thread_id = enclave_id + (size_enclave_metadata * PAGE_SIZE);
  uint64_t timer_limit = 0xeffffffffff;

  printm("Thread Load\n");

  result = sm_thread_load(enclave_id, thread_id, EVBASE, 0x0, timer_limit); // SP is set by the enclave itself
  if(result != MONITOR_OK) {
    printm("sm_thread_load FAILED with error code %d\n\n", result);
    test_completed();
  }

  printm("Enclave Init\n");

  result = sm_enclave_init(enclave_id);
  if(result != MONITOR_OK) {
    printm("sm_enclave_init FAILED with error code %d\n\n", result);
    test_completed();
  }

  printm("Enclave Enter\n");

  result = sm_enclave_enter(enclave_id, thread_id);

  send_exit_cmd(0);
  test_completed();
}
