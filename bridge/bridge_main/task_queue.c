#include <bridge/bridge_main/task_queue.h>

using std::deque;
using std::shared_ptr;
extern std::shared_ptr<fsm_table_t> sp_g_fsm_table;

static deque< shared_ptr<task_node_t> > waiting_task;
static deque< shared_ptr<task_node_t> > doing_task;