#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include <direct.h>

// 全局变量：仓库数量，用于生成新仓库的命名编号
int warehouse_number = 0;

// 结构体：食物信息
typedef struct food {
    char food_name[100];  // 食物名称，最大长度99个字符
    char food_type[100];  // 食物种类（如蔬菜、肉类、水果）
    int food_volume;  // 食物体积
    int food_temperature;  // 食物保存的温度
} food;

// 结构体：链表节点
typedef struct node {
    food data;  // 数据域，一个food类型的变量
    struct node* next;  // 指针域，指向下一个节点
} node;

// 结构体：冰柜信息
typedef struct frezzer {
    node* head;  // 头指针
    node* tail;  // 尾指针
    int frezzer_temperature;  // 冰柜的温度
    int frezzer_available_volume;  // 冰柜的可用容积
} frezzer;

/*
 * 函数：frezzer_init
 * 功能：初始化冰柜结构体，设置初始值
 * 参数：f - 指向需要初始化的冰柜结构体的指针
 */
void frezzer_init(frezzer* f) {  // 初始化冰柜结构体
    f->head = NULL;  // 初始化头指针为空
    f->tail = NULL;  // 初始化尾指针为空
    f->frezzer_temperature = 10;  // 最高允许温度10度
    f->frezzer_available_volume = 100;  // 可用容积最大值100
}

node* create_node(node** head, node** tail) {  //新建链节并尾插，传入head和tail的二级指针
    node* temp = (node*)malloc(sizeof(node)); // 新建节点
    if (*head == NULL) {
        // 如果链表为空，head和tail指向新节点
        *head = temp;
        *tail = temp;
    } else {
        // 若链表不空，则尾插，并更新tail
        (*tail)->next = temp;
        *tail = temp;
    }

    // 初始化新节点的数据域
    strcpy(temp->data.food_name, "NONE");
    temp->data.food_temperature = 0;
    strcpy(temp->data.food_type, "NONE");
    temp->data.food_volume = 0;
    temp->next = NULL;

    return temp;  // 返回指向新节点的指针
}


void free_list(node* head) {  //释放整个链表，不释放头指针（在frezzer里），传入头指针
    for (node *current = head, *next; current != NULL; current = next) { // 当前处理的节点
        next = current->next;  // 保存下一个节点的地址
        free(current);  // 释放当前节点
    }
}

void calculate_freezer_status(frezzer* f) {  // 计算冰柜的剩余容积和温度，每次更改冰柜中食物调用 传入指向冰柜的指针 无返
    int used_volume = 0;  // 已使用的容积
    int min_temp = 10;  // 最低所需温度，初值为10
    int has_food = 0;  // 标记冰柜中是否有食物

    for (node* temp = f->head; temp != NULL; temp = temp->next) { // 遍历指针
        has_food = 1;
        used_volume += temp->data.food_volume; // 累加体积
        // 找到所有食物中要求的最低温度
        if (temp->data.food_temperature < min_temp) {
            min_temp = temp->data.food_temperature;
        }
    }

    // 更新剩余容积
    f->frezzer_available_volume = 100 - used_volume;
    
    // 更新冰柜温度：如果有食物则为最低所需温度，否则为默认10度
    if (has_food) {
        f->frezzer_temperature = min_temp;
    } else {
        f->frezzer_temperature = 10;
    }
}

/*
 * 函数：sort_food_list
 * 功能：对冰柜中的食物链表按体积进行降序排序（冒泡排序）
 * 参数：f - 指向冰柜结构体的指针
 */
void sort_food_list(frezzer* f) {
    if (f->head == NULL || f->head->next == NULL) return; // 空链表或单节点无需排序

    int swapped;    // 变量：标记本轮是否发生了交换
    node* ptr1;     // 变量：用于遍历的指针
    node* lptr = NULL; // 变量：指向已排序部分的开始位置

    for (swapped = 1; swapped; ) {
        swapped = 0;
        
        for (ptr1 = f->head; ptr1->next != lptr; ptr1 = ptr1->next) { // 变量：用于遍历的指针
            // 如果当前节点体积小于下一个节点体积，则交换数据（降序）
            if (ptr1->data.food_volume < ptr1->next->data.food_volume) {
                food temp = ptr1->data;
                ptr1->data = ptr1->next->data;
                ptr1->next->data = temp;
                swapped = 1;
            }
        }
        lptr = ptr1; // 更新已排序边界
    }
}

/*
 * 函数：save_freezer_to_file
 * 功能：将冰柜中的数据保存到指定文件
 * 参数：filepath - 文件路径
 * 参数：f - 指向冰柜结构体的指针
 */
void save_freezer_to_file(const char* filepath, frezzer* f) {
    FILE* fp = fopen(filepath, "w"); // 以写模式打开文件
    if (fp == NULL) {
        printf("Error: Cannot save file %s\n", filepath);
        return;
    }

    // 遍历链表，将每个食物的信息写入文件
    for (node* curr = f->head; curr != NULL; curr = curr->next) {
        fprintf(fp, "%s %s %d %d\n", 
            curr->data.food_name, 
            curr->data.food_type, 
            curr->data.food_volume, 
            curr->data.food_temperature);
    }
    fclose(fp); // 关闭文件
}

/*
 * 函数：load_freezer_from_file
 * 功能：从指定文件读取数据并加载到冰柜结构体中
 * 参数：filepath - 文件路径
 * 参数：f - 指向冰柜结构体的指针
 */
void load_freezer_from_file(const char* filepath, frezzer* f) {
    frezzer_init(f); // 先初始化冰柜（清空旧数据）
    FILE* fp = fopen(filepath, "r"); // 以读模式打开文件
    if (fp == NULL) {
        // 如果文件不存在（可能是新建的），直接返回
        return;
    }

    char name[100], type[100]; // 临时变量：存储读取的名称和类型
    int vol, temp;             // 临时变量：存储读取的体积和温度

    // 循环读取文件内容，每次读取4项数据
    for (; fscanf(fp, "%s %s %d %d", name, type, &vol, &temp) == 4; ) {
        node* new_node = create_node(&(f->head), &(f->tail)); // 创建新节点
        strcpy(new_node->data.food_name, name);
        strcpy(new_node->data.food_type, type);
        new_node->data.food_volume = vol;
        new_node->data.food_temperature = temp;
    }
    fclose(fp); // 关闭文件
    
    // 读取完成后重新计算冰柜状态并排序
    calculate_freezer_status(f);
    sort_food_list(f);
}

/*
 * 函数：show_maininterface
 * 功能：显示一级菜单（仓库列表），并提示用户操作
 */
void show_maininterface() {
    printf("\n=== Main Menu ===\n");
    printf("Welcome to llwwds' freezer management system\n");
    printf("\n");

    warehouse_number = 0;  // 重置仓库计数
    DIR *dir = opendir("data"); // 打开data目录
    if (dir) {
        struct dirent *entry; // 目录项指针
        for (entry = readdir(dir); entry != NULL; entry = readdir(dir)) {
            struct stat st; // 文件状态结构体
            char path[512]; // 路径缓冲区
            sprintf(path, "data/%s", entry->d_name);
            
            // 检查是否为目录且不是 . 或 ..
            if (stat(path, &st) == 0 && S_ISDIR(st.st_mode) &&
                strcmp(entry->d_name, ".") != 0 &&
                strcmp(entry->d_name, "..") != 0) {
                
                // 尝试解析仓库编号以更新全局计数器
                int num;
                if (sscanf(entry->d_name, "warehouse_%d", &num) == 1) {
                    if (num > warehouse_number) warehouse_number = num;
                }
                printf("  [Warehouse] %s\n", entry->d_name); // 显示仓库名
            }
        }
        closedir(dir); // 关闭目录
    }

    // Show options
    printf("\n");
    printf("========== Main Menu ==========\n");
    printf(" [0] Create Warehouse\n");
    printf(" [1] Open Warehouse\n");
    printf(" [2] Delete Warehouse\n");
    printf(" [-1] Exit\n");
    printf("===============================\n");
    printf("Please enter a number to operate: ");
}

/*
 * 函数：show_inside_warehoues
 * 功能：显示二级菜单（冰柜列表），列出指定仓库内的所有冰柜
 * 参数：target_warehouse_path - 目标仓库的路径
 */
void show_inside_warehoues(char* target_warehouse_path) {
    printf("\n=== Warehouse: %s ===\n", target_warehouse_path);
    struct stat st;
    // 检查仓库路径是否有效
    if (stat(target_warehouse_path, &st) != 0 || !S_ISDIR(st.st_mode)) {
        printf("The warehouse does not exist.\n");
        return;
    }

    DIR *dir = opendir(target_warehouse_path); // 打开仓库目录
    int count = 0; // 变量：记录冰柜数量
    if (dir) {
        struct dirent *entry;
        for (entry = readdir(dir); entry != NULL; entry = readdir(dir)) {
            struct stat st;
            char path[512];
            sprintf(path, "%s/%s", target_warehouse_path, entry->d_name);
            // 检查是否为普通文件
            if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
                char *dot = strrchr(entry->d_name, '.');
                // 检查后缀名是否为 .txt
                if (dot && strcmp(dot, ".txt") == 0) {
                    frezzer f;
                    load_freezer_from_file(path, &f);
                    printf("  [Freezer] %s  Temperature: %d C  Available: %d / 100\n",
                           entry->d_name, f.frezzer_temperature, f.frezzer_available_volume);
                    count++;
                }
            }
        }
        closedir(dir);
    }
    if (count == 0) printf("  (Empty)\n"); // 如果没有冰柜，提示为空

    // 显示操作选项
    printf("\n");
    printf("Enter 0 to create a new freezer\n");
    printf("Enter 1 to open a freezer\n");
    printf("Enter 2 to delete a freezer\n");
    printf("Enter -1 to return\n");
    printf("Please enter a number to operate: ");
}

/*
 * 函数：show_freezer_content
 * 功能：显示三级菜单（食物列表），列出冰柜内的食物信息
 * 参数：f - 指向冰柜结构体的指针
 * 参数：freezer_name - 冰柜的名称
 */
void show_freezer_content(frezzer* f, const char* freezer_name) {
    printf("\n=== Freezer: %s ===\n", freezer_name);
    calculate_freezer_status(f); // 确保显示前状态是最新的
    printf("Temperature: %d C\n", f->frezzer_temperature);
    printf("Available Volume: %d / 100\n", f->frezzer_available_volume);
    printf("Food List (Sorted by Volume Desc):\n");
    printf("%-20s %-10s %-10s %-10s\n", "Name", "Type", "Volume", "Temp");
    printf("----------------------------------------------------\n");
    
    int idx = 0;          // 变量：食物序号
    for (node* curr = f->head; curr != NULL; curr = curr->next) { // 遍历指针
        printf("%d. %-17s %-10s %-10d %-10d\n", ++idx, curr->data.food_name, curr->data.food_type, curr->data.food_volume, curr->data.food_temperature);
    }

    // Show options
    printf("\n");
    printf("========== Freezer Menu ==========\n");
    printf(" [0] Add Food\n");
    printf(" [1] Delete Food\n");
    printf(" [2] Modify Food\n");
    printf(" [3] Query Food\n");
    printf(" [-1] Return\n");
    printf("==================================\n");
    printf("Please enter a number to operate: ");
}

/*
 * 函数：remove_dir_recursive
 * 功能：递归删除目录及其包含的所有文件和子目录
 * 参数：path - 要删除的目录路径
 */
void remove_dir_recursive(const char *path) {
    DIR *d = opendir(path); // 打开目录
    size_t path_len = strlen(path);
    struct dirent *p;

    if (!d) return; // 打开失败直接返回

    for (p = readdir(d); p != NULL; p = readdir(d)) {
        char *buf;
        size_t len;

        // 跳过 . 和 ..
        if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
            continue;

        len = path_len + strlen(p->d_name) + 2;
        buf = malloc(len); // 分配路径缓冲区内存

        if (buf) {
            snprintf(buf, len, "%s/%s", path, p->d_name);
            struct stat st;
            if (stat(buf, &st) == 0) {
                // 如果是目录则递归删除，否则直接删除文件
                if (S_ISDIR(st.st_mode))
                    remove_dir_recursive(buf);
                else
                    remove(buf);
            }
            free(buf); // 释放缓冲区
        }
    }
    closedir(d); // 关闭目录
    _rmdir(path); // 删除空目录本身
}

/*
 * Function: clear_buffer
 * Purpose: Clear the input buffer to prevent infinite loops on invalid input
 */
void clear_buffer() {
    int c;
    for (c = getchar(); c != '\n' && c != EOF; c = getchar());
}

/*
 * 函数：main
 * 功能：程序入口，主循环控制各级菜单的切换
 */
int main() {
    // 常量：定义菜单层级的状态码
    const int maininterface_menu = 1;  // 一级菜单：仓库管理
    const int inside_warehouse_menu = 2; // 二级菜单：冰柜管理
    const int inside_frezzer_menu = 3;   // 三级菜单：食物管理
    
    int current_menu = maininterface_menu; // 变量：当前所处的菜单层级
    int choice = 0; // 变量：用户输入的选项
    char target_warehouse_path[256];  // 变量：当前选中的仓库路径
    char target_freezer_path[512];    // 变量：当前选中的冰柜文件路径
    char current_freezer_name[100];   // 变量：当前选中的冰柜名称

    frezzer current_frezzer; // 变量：当前操作的冰柜对象
    frezzer_init(&current_frezzer); // 初始化冰柜对象

    for (;;) {
        // === 一级菜单逻辑 ===
        if (current_menu == maininterface_menu) {
            show_maininterface(); // 显示一级菜单
            scanf("%d", &choice); // 读取用户输入

            if (choice == -1) {
                return 0; // 退出程序
            } else if (choice == 0) {
                // 创建新仓库
                warehouse_number++; // 编号加1
                char new_path[256];
                sprintf(new_path, "data/warehouse_%d", warehouse_number);
                if (_mkdir(new_path) == 0) {
                    printf("Created warehouse_%d successfully.\n", warehouse_number);
                } else {
                    printf("Failed to create warehouse (might already exist).\n");
                }
            } else if (choice == 1) {
                // 打开仓库
                printf("\nEnter the number of the warehouse to open: ");
                int temp;
                if (scanf("%d", &temp) == 1) {
                    sprintf(target_warehouse_path, "data/warehouse_%d", temp);
                    struct stat st;
                    if (stat(target_warehouse_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                        current_menu = inside_warehouse_menu; // 切换到二级菜单
                    } else {
                        printf("Warehouse not found!\n");
                    }
                }
            } else if (choice == 2) {
                // 删除仓库
                printf("\nEnter the number of the warehouse to delete: ");
                int temp;
                if (scanf("%d", &temp) == 1) {
                    clear_buffer();
                    char path[256];
                    sprintf(path, "data/warehouse_%d", temp);
                    printf("Deleting %s...\n", path);
                    remove_dir_recursive(path); // 递归删除
                }
            }
        } 
        // === 二级菜单逻辑 ===
        else if (current_menu == inside_warehouse_menu) {
            show_inside_warehoues(target_warehouse_path); // 显示二级菜单
            scanf("%d", &choice);

            if (choice == -1) {
                current_menu = maininterface_menu; // 返回一级菜单
            } else if (choice == 0) {
                // 新建冰柜
                printf("Enter number for new freezer: ");
                int num;
                if (scanf("%d", &num) == 1) {
                    clear_buffer();
                    char path[512];
                    sprintf(path, "%s/frezzer%d.txt", target_warehouse_path, num);
                    FILE* fp = fopen(path, "w");
                    if (fp) {
                        fclose(fp);
                        printf("Freezer created: frezzer%d\n", num);
                    } else {
                        printf("Failed to create freezer.\n");
                    }
                } else {
                    clear_buffer();
                    printf("Invalid number.\n");
                }
            } else if (choice == 1) {
                // 打开冰柜
                printf("Enter freezer name to open (without .txt): ");
                char name[100];
                scanf("%s", name);
                sprintf(target_freezer_path, "%s/%s.txt", target_warehouse_path, name);
                struct stat st;
                if (stat(target_freezer_path, &st) == 0) {
                    strcpy(current_freezer_name, name);
                    load_freezer_from_file(target_freezer_path, &current_frezzer); // 读取数据
                    current_menu = inside_frezzer_menu; // 切换到三级菜单
                } else {
                    printf("Freezer not found.\n");
                }
            } else if (choice == 2) {
                // 删除冰柜
                printf("Enter number of freezer to delete: ");
                int num;
                if (scanf("%d", &num) == 1) {
                    clear_buffer();
                    char path[512];
                    sprintf(path, "%s/frezzer%d.txt", target_warehouse_path, num);
                    if (remove(path) == 0) printf("Deleted: frezzer%d\n", num);
                    else printf("Delete failed.\n");
                } else {
                    clear_buffer();
                    printf("Invalid number.\n");
                }
            }
        }
        // === 三级菜单逻辑 ===
        else if (current_menu == inside_frezzer_menu) {
            show_freezer_content(&current_frezzer, current_freezer_name); // 显示三级菜单
            if (scanf("%d", &choice) != 1) choice = -999; clear_buffer();

            if (choice == -1) {
                // 返回上一级并保存数据
                save_freezer_to_file(target_freezer_path, &current_frezzer);
                free_list(current_frezzer.head); // 释放内存
                current_menu = inside_warehouse_menu; // 返回二级菜单
            } else if (choice == 0) {
                // 添加食物
                food new_food;
                printf("Enter Name: "); scanf("%s", new_food.food_name); clear_buffer();
                printf("Enter Type (Veg/Meat/Fruit): "); scanf("%s", new_food.food_type); clear_buffer();
                printf("Enter Volume: "); if(scanf("%d", &new_food.food_volume)!=1) new_food.food_volume=0; clear_buffer();
                printf("Enter Temp: "); if(scanf("%d", &new_food.food_temperature)!=1) new_food.food_temperature=0; clear_buffer();

                // 检查约束条件
                calculate_freezer_status(&current_frezzer); // 更新当前状态
                
                // 1. 体积检查
                if (new_food.food_volume > current_frezzer.frezzer_available_volume) {
                    printf("Error: Not enough space! Available: %d, Needed: %d\n", 
                           current_frezzer.frezzer_available_volume, new_food.food_volume);
                    continue;
                }

                // 2. 温度检查
                if (new_food.food_temperature < -20) {
                    printf("Error: Temperature too low! Min allowed is -20.\n");
                    continue;
                }
                if (new_food.food_temperature > 10) {
                     printf("Error: Temperature too high! Max allowed is 10.\n");
                     continue;
                }

                // 添加到链表
                node* new_node = create_node(&(current_frezzer.head), &(current_frezzer.tail));
                new_node->data = new_food;
                printf("Food added.\n");
                
                // 重新计算并排序
                calculate_freezer_status(&current_frezzer);
                sort_food_list(&current_frezzer);

            } else if (choice == 1) {
                // 删除食物
                printf("Enter index to delete: ");
                int idx;
                if(scanf("%d", &idx)!=1) idx=-1; clear_buffer();
                if (idx < 1) continue;

                int count = 1;
                // 寻找指定序号的节点
                int found_idx = 0;
                for (node *curr = current_frezzer.head, *prev = NULL; curr != NULL; prev = curr, curr = curr->next, count++) {
                   if (count == idx) {
                        if (prev == NULL) { // 删除的是头节点
                            current_frezzer.head = curr->next;
                            if (current_frezzer.head == NULL) current_frezzer.tail = NULL;
                        } else {
                            prev->next = curr->next;
                            if (prev->next == NULL) current_frezzer.tail = prev;
                        }
                        free(curr); // 释放节点内存
                        printf("Deleted.\n");
                        calculate_freezer_status(&current_frezzer);
                        found_idx = 1;
                        break;
                   }
                }
                if (!found_idx) {
                    printf("Invalid index.\n");
                }
            } else if (choice == 2) {
                // 修改食物
                printf("Enter index to modify: ");
                int idx;
                if(scanf("%d", &idx)!=1) idx=-1; clear_buffer();
                
                int count = 1;
                int found_idx = 0;
                for (node* curr = current_frezzer.head; curr != NULL; curr = curr->next, count++) {
                    if (count == idx) {
                        food temp_food = curr->data;
                        printf("Modifying %s. Enter new details.\n", temp_food.food_name);
                        
                        printf("New Name: "); scanf("%s", temp_food.food_name);
                        printf("New Type: "); scanf("%s", temp_food.food_type);
                        printf("New Volume: "); scanf("%d", &temp_food.food_volume);
                        printf("New Temp: "); scanf("%d", &temp_food.food_temperature);
                        
                        // 验证修改后的约束条件
                        int current_used = 100 - current_frezzer.frezzer_available_volume;
                        int other_used = current_used - curr->data.food_volume;
                        int new_avail = 100 - other_used;
                        
                        if (temp_food.food_volume > new_avail) {
                            printf("Error: Not enough space for modification.\n");
                        } else if (temp_food.food_temperature < -20 || temp_food.food_temperature > 10) {
                            printf("Error: Invalid temperature.\n");
                        } else {
                            curr->data = temp_food; // 更新数据
                            printf("Modified.\n");
                            calculate_freezer_status(&current_frezzer);
                            sort_food_list(&current_frezzer);
                        }
                        found_idx = 1;
                        break;
                    }
                }
                
                if (!found_idx) {
                    printf("Invalid index.\n");
                }
            } else if (choice == 3) {
                // 查询特定种类的食物
                printf("Enter food type to query: ");
                char q_type[100];
                scanf("%s", q_type); clear_buffer();
                printf("\nMatches for '%s':\n", q_type);
                int found = 0;
                for (node* curr = current_frezzer.head; curr != NULL; curr = curr->next) {
                    if (strcmp(curr->data.food_type, q_type) == 0) {
                         printf("  %s (Vol: %d, Temp: %d)\n", curr->data.food_name, curr->data.food_volume, curr->data.food_temperature);
                         found = 1;
                    }
                }
                if (!found) printf("  None found.\n");
                printf("\nPress 1 to continue...");
                int dummy; scanf("%d", &dummy);
            }
        }
    }
    return 0;
}
