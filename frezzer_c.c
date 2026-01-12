#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <direct.h>

int warehouse_number = 0;  // 全局变量：仓库数量，用于生成新仓库的命名编号

// 结构体：食物信息
typedef struct food {
    char food_name[100];  // 食物名称，最大长度99个字符
    char food_type[100];  // 食物种类（如蔬菜、肉类、水果）
    int food_volume;  // 食物体积
    int food_temperature;  // 食物保存的温度
}food;

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

void frezzer_init(frezzer* f) {  // 初始化冰柜结构体
    f->head = NULL;  // 初始化头指针为空
    f->tail = NULL;  // 初始化尾指针为空
    f->frezzer_temperature = 10;  // 最高允许温度10度
    f->frezzer_available_volume = 100;  // 可用容积最大值100
}

node* create_node(node** head, node** tail) {  //新建链节并尾插，传入head和tail的二级指针
    node* temp = (node*)malloc(sizeof(node)); // 新建节点
    temp->next = NULL; // 仅初始化指针

    if (*head == NULL) {
        // 如果链表为空，head和tail指向新节点
        *head = temp;
        *tail = temp;
    } else {
        // 若链表不空，则尾插，并更新tail
        (*tail)->next = temp;
        *tail = temp;
    }

    return temp;  // 返回指向新节点的指针
}


void free_list(node* head) {  //释放整个链表，不释放头指针（在frezzer里），传入头指针
    for (node *temp = head, *next; temp != NULL; temp = next) { // 当前处理的节点
        next = temp->next;  // 保存下一个节点的地址
        free(temp);  // 释放当前节点
    }
}

void calculate_freezer_status(frezzer* f) {  // 计算冰柜的剩余容积和温度，更改冰柜中食物时调用 传入指向冰柜的指针 无返
    int used_volume = 0;  // 记录已使用的容积
    int min_temp = 10;  // 记录最低温度，初值为10

    for (node* temp = f->head; temp != NULL; temp = temp->next) { // 遍历冰柜
        used_volume += temp->data.food_volume; // 累加体积
        if (temp->data.food_temperature < min_temp) {  // 求最小温度
            min_temp = temp->data.food_temperature;
        }
    }

    f->frezzer_available_volume = 100 - used_volume;  // 更新冰柜体积
    f->frezzer_temperature=min_temp;// 更新冰柜温度
}

int cmp(const void *a, const void *b) {  // qsort排序单链表用的排序函数
    const food *food_a = a, *food_b = b;
    return food_b->food_volume - food_a->food_volume; // 降序排序
}

void sort_food_list(frezzer* f) {  //对冰柜中的食物按照体积进行 降序排序 传入指向冰柜变量的指针
    if (f->head == NULL || f->head->next == NULL) return; // 若为空链表/单节点，不用排序

    food temp_data[100];  // 暂时记录链表中全部数据
    int tag = 0;  // 临时变量，用两次

    // 1. 复制链表数据到数组（无初始化）
    for (node* temp = f->head; temp != NULL; temp = temp->next) {
        temp_data[tag++] = temp->data;
    }

    // 2. 用实际节点数 tag 替代固定 100
    qsort(temp_data, tag, sizeof(food), cmp);

    // 3. 复制回链表
    tag = 0;
    for (node* temp = f->head; temp != NULL; temp = temp->next) {
        temp->data = temp_data[tag++];
    }
}

void save_freezer_to_file(char* filepath, frezzer* f) {  // 将链表中的内容写入文件，传入：文件路径 指向冰柜结构体的指针
    FILE* fp = fopen(filepath, "w");  // 以写模式打开文件
    if (fp == NULL) {  // 若找不到，则报错并返回
        printf("Error: Cannot save file %s\n", filepath);
        return;
    }

    for (node* temp = f->head; temp != NULL; temp = temp->next) {  // 遍历链表写入文件，文件中的顺序为：名字 类型 体积 温度\n
        fprintf(fp, "%s %s %d %d\n", 
            temp->data.food_name, 
            temp->data.food_type, 
            temp->data.food_volume, 
            temp->data.food_temperature);
    }
    fclose(fp);  // 关闭文件
}

void load_freezer_from_file(char* filepath, frezzer* f) {  // 读取指定文件，生成链表，并加载数据到链表中，传入：文件路径 指向冰柜结构体的指针
    frezzer_init(f); // 先初始化冰柜
    FILE* fp = fopen(filepath, "r"); // 以读模式打开文件，FILE为读取文件用的数据类型
    if (fp == NULL) {
        // 如果文件不存在，则报错并返回
        printf("Error: Cannot find file %s\n", filepath);
        return;
    }

    char name[100], type[100];  // 记录读到的名称和类型
    int vol, temp;  // 记录读到的体积和温度
    for (; fscanf(fp, "%s %s %d %d", name, type, &vol, &temp) == 4; ) {  // 循环读取文件内容，每次读取4项（一行）数据
        node* new_node = create_node(&(f->head), &(f->tail)); // 创建新节点，接下来读取并将内容写入新节点中
        strcpy(new_node->data.food_name, name);
        strcpy(new_node->data.food_type, type);
        new_node->data.food_volume = vol;
        new_node->data.food_temperature = temp;
    }
    fclose(fp); // 关闭文件
    
    calculate_freezer_status(f);  // 读取完成后重新计算冰柜状态并排序
    sort_food_list(f);
}

void show_maininterface() {  // 显示一级菜单
    printf("\n=== Main Menu ===\n");
    printf("Welcome to llwwds' freezer management system\n");
    printf("\n");

    warehouse_number = 0;  // 重置仓库计数
    int max_num = 0; // 用局部变量替代全局计数器
    DIR *dir = opendir("data"); // 打开data目录，DIR为读取文件用的数据类型，若失败则返回NULL
    if (dir!=NULL) {
        for (struct dirent *temp = readdir(dir); temp != NULL; temp = readdir(dir)) {
            struct stat st;  // 文件状态结构体
            char path[600];  // 路径缓冲区
            sprintf(path, "data/%s", temp->d_name);
            
            if (stat(path, &st) == 0 && S_ISDIR(st.st_mode) && strcmp(temp->d_name, ".") != 0 && strcmp(temp->d_name, "..") != 0) {
                // 检查当前查看的文件是否 不为文件夹，不为当前文件，不为上一级文件 若满足条件则读它的编号
                int num;
                if (sscanf(temp->d_name, "warehouse_%d", &num) == 1 && num > max_num) {
                    max_num = num;
                }
                printf("  [Warehouse] %s\n", temp->d_name); // 打印仓库名
            }
        }
        closedir(dir); // 关闭目录
        warehouse_number = max_num; // 仅更新一次
    }
    else{
        printf("Error: Cannot open data directory\n");  // 若失败，则报错并返回
        return;
    }

    printf("\n");  // 显示文件名之后，打印操作提示
    printf("========== Main Menu ==========\n");
    printf(" [0] Create Warehouse\n");
    printf(" [1] Open Warehouse\n");
    printf(" [2] Delete Warehouse\n");
    printf(" [-1] Exit\n");
    printf("===============================\n");
    printf("Please enter a number to operate: ");
}

void show_inside_warehoues(char* target_warehouse_path) {  // 显示二级菜单（仓库内的冰柜们），列出指定仓库内的所有冰柜 传入：仓库路径
    printf("\n=== Warehouse: %s ===\n", target_warehouse_path);
    struct stat temp;  // 存放文件夹的属性信息
    if (stat(target_warehouse_path, &temp) != 0 || !S_ISDIR(temp.st_mode)) {  // 若无法打开，则报错并返回
        printf("The warehouse does not exist.\n");
        return;
    }

    DIR *dir = opendir(target_warehouse_path); // 打开目标仓库的文件夹
    int count = 0; // 变量：记录冰柜数量
    if (dir) {
        struct dirent *entry;  // 指向目录项的指针
        for (entry = readdir(dir); entry != NULL; entry = readdir(dir)) {  // 遍历目标仓库的文件夹中的所有文件，直到下一个是null
            struct stat temp;
            char path[600];
            sprintf(path, "%s/%s", target_warehouse_path, entry->d_name);  // 拼出完整的文件路径
            if (stat(path, &temp) == 0 && S_ISREG(temp.st_mode)) {  // 判断文件是否存在，并检查是否为普通文件
                char *dot = strrchr(entry->d_name, '.');  // 检查后缀名是否为 .txt，dot指向这个位置
                if (dot && strcmp(dot, ".txt") == 0) {
                    frezzer f;  // 新建一个冰柜变量
                    load_freezer_from_file(path, &f);  // 初始化冰柜
                    printf("  [Freezer] %s  Temperature: %d C  Available: %d / 100\n",entry->d_name, f.frezzer_temperature, f.frezzer_available_volume);
                    count++;
                }
            }
        }
        closedir(dir);  // 关闭目标仓库的文件夹
    }
    else{
        printf("Error: Cannot open warehouse directory\n");  // 若失败，则报错并返回
        return;
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
            sprintf(buf, "%s/%s", path, p->d_name);
            struct stat temp;
            if (stat(buf, &temp) == 0) {
                // 如果是目录则递归删除，否则直接删除文件
                if (S_ISDIR(temp.st_mode))
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

// -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int main() {
    // 常量：定义菜单层级的状态码
    const int maininterface_menu = 1;  // 一级菜单：仓库管理
    const int inside_warehouse_menu = 2; // 二级菜单：冰柜管理
    const int inside_frezzer_menu = 3;   // 三级菜单：食物管理
    
    int current_menu = maininterface_menu; // 记录当前所处的菜单层级
    int choice = 0; // 记录用户输入的选项
    char target_warehouse_path[600];  // 记录当前选中的仓库路径
    char target_freezer_path[600];    // 记录当前选中的冰柜文件路径
    char current_freezer_name[100];   // 记录当前选中的冰柜名称

    frezzer current_frezzer; // 记录当前操作的冰柜
    frezzer_init(&current_frezzer); // 初始化冰柜

    for (;;) {  // 死循环：持续处理用户输入，直到用户选择退出
        if (current_menu == maininterface_menu) {
            show_maininterface(); // 显示一级菜单
            scanf("%d", &choice); // 读取用户输入

            if (choice == -1) {  // 退出程序
                return 0;
            } else if (choice == 0) {  // 创建新仓库
                warehouse_number++;  // 更新仓库计数
                char new_path[600];
                sprintf(new_path, "data/warehouse_%d", warehouse_number);  // 拼出新仓库的路径
                if (_mkdir(new_path) == 0) {  // 新建仓库文件夹
                    printf("Created warehouse_%d successfully.\n", warehouse_number);  // 若创建成功，打印成功的提示
                } else {
                    printf("Failed to create warehouse (might already exist).\n");  // 若创建失败，报错
                }
            } else if (choice == 1) {  // 打开仓库
                printf("\nEnter the number of the warehouse to open: ");  // 提示用户输入仓库编号
                int temp;  // 记录用户输入的仓库编号
                if (scanf("%d", &temp) == 1) {
                    sprintf(target_warehouse_path, "data/warehouse_%d", temp);
                    struct stat temp;
                    if (stat(target_warehouse_path, &temp) == 0 && S_ISDIR(temp.st_mode)) {
                        current_menu = inside_warehouse_menu; // 切换到二级菜单
                    } else {
                        printf("Warehouse not found!\n");
                    }
                }
            } else if (choice == 2) {  // 删除仓库
                printf("\nEnter the number of the warehouse to delete: ");  // 提示用户输入仓库编号
                int temp;  // 记录用户输入的仓库编号
                if (scanf("%d", &temp) == 1) {
                    clear_buffer();
                    char path[600];
                    sprintf(path, "data/warehouse_%d", temp);
                    printf("Deleting data/warehouse_%d...\n", temp);
                    remove_dir_recursive(path); // 递归删除
                }
            }
        } 
        // === 二级菜单逻辑 ===
        else if (current_menu == inside_warehouse_menu) {
            show_inside_warehoues(target_warehouse_path); // 显示二级菜单
            while (scanf("%d", &choice) != 1) {
                clear_buffer();
                printf("Invalid input. Please enter a number.\n");
            }

            if (choice == -1) {  // 返回一级菜单
                current_menu = maininterface_menu;
            } else if (choice == 0) {  // 新建冰柜
                printf("\nEnter number for new freezer: ");  // 提示用户输入冰柜编号
                int num;
                if (scanf("%d", &num) == 1) {
                    clear_buffer();
                    char path[600];
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
            } else if (choice == 1) {  // 打开冰柜
                printf("\nEnter freezer name to open (without .txt): ");  // 提示用户输入冰柜名称
                char name[100];
                scanf("%s", name);
                sprintf(target_freezer_path, "%s/%s.txt", target_warehouse_path, name);
                struct stat temp;
                if (stat(target_freezer_path, &temp) == 0) {
                    strcpy(current_freezer_name, name);
                    load_freezer_from_file(target_freezer_path, &current_frezzer); // 读取数据
                    current_menu = inside_frezzer_menu; // 切换到三级菜单
                } else {
                    printf("Freezer not found.\n");
                }
            } else if (choice == 2) {  // 删除冰柜
                printf("\nEnter number of freezer to delete: ");  // 提示用户输入冰柜编号
                int num;
                if (scanf("%d", &num) == 1) {
                    clear_buffer();
                    char path[600];
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
            for (choice = scanf("%d", &choice); choice != 1; choice = scanf("%d", &choice)) {
                clear_buffer();
             }

            if (choice == -1) {  // 返回上一级并保存数据
                save_freezer_to_file(target_freezer_path, &current_frezzer);
                free_list(current_frezzer.head); // 释放内存
                current_menu = inside_warehouse_menu; // 返回二级菜单
            } else if (choice == 0) {  // 添加食物
                food new_food;
                printf("\nEnter Name: "); scanf("%s", new_food.food_name); clear_buffer();  // 提示用户输入食物名称
                printf("Enter Type (Veg/Meat/Fruit): "); scanf("%s", new_food.food_type); clear_buffer();  // 提示用户输入食物类型
                printf("Enter Volume: "); if(scanf("%d", &new_food.food_volume)!=1) new_food.food_volume=0; clear_buffer();  // 提示用户输入食物体积
                printf("Enter Temp: "); if(scanf("%d", &new_food.food_temperature)!=1) new_food.food_temperature=0; clear_buffer();  // 提示用户输入食物温度

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

            } else if (choice == 1) {  // 删除食物
                printf("\nEnter index to delete: ");  // 提示用户输入要删除的食物序号
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
            } else if (choice == 2) {  // 修改食物
                printf("\nEnter index to modify: ");  // 提示用户输入要修改的食物序号
                int idx;
                if(scanf("%d", &idx)!=1) idx=-1; clear_buffer();
                
                int count = 1;
                int found_idx = 0;
                for (node* curr = current_frezzer.head; curr != NULL; curr = curr->next, count++) {
                    if (count == idx) {
                        food temp_food = curr->data;
                        printf("Modifying %s. Enter new details.\n", temp_food.food_name);
                        
                        printf("\nNew Name: "); scanf("%s", temp_food.food_name); clear_buffer();  // 提示用户输入新的食物名称
                        printf("New Type: "); scanf("%s", temp_food.food_type); clear_buffer();  // 提示用户输入新的食物类型
                        printf("New Volume: "); if(scanf("%d", &temp_food.food_volume)!=1) temp_food.food_volume=0; clear_buffer();  // 提示用户输入新的食物体积
                        printf("New Temp: "); if(scanf("%d", &temp_food.food_temperature)!=1) temp_food.food_temperature=0; clear_buffer();  // 提示用户输入新的食物温度
                        
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
            } else if (choice == 3) {  // 查询特定种类的食物
                printf("\nEnter food type to query: ");  // 提示用户输入要查询的食物类型
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