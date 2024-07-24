#include "list.h"
#include "hash.h"
#include "bitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main()  { 
	srand(time(NULL));
	LIST* list = (LIST*)malloc(sizeof(LIST) * 10); 
	HASH* hash = (HASH*)malloc(sizeof(HASH) * 10); 
	BITMAP *bitmap = (BITMAP *)malloc(sizeof(BITMAP) * 10); 
	char arg[200];
	char** cmd = (char**)malloc(sizeof(char*) * 200);
	while (1) {
		fgets(arg, 200, stdin);
		arg[strlen(arg) - 1] = '\0';
		char* strtmp = strtok(arg, " ");
		int idx = 0;
		while (strtmp != NULL) {
			cmd[idx] = strtmp;
			strtmp = strtok(NULL, " ");
			idx++;
		}

		//quit
		if (!strcmp(cmd[0], "quit")) {
			exit(1);
		}
		//create 
		else if (!strcmp(cmd[0], "create")) {
			if (!strcmp(cmd[1], "list")) {
				char* tmp = cmd[2];
				int list_idx = tmp[strlen(tmp) - 1] - '0';
				list_init(&list[list_idx]);
			}
			else if (!strcmp(cmd[1], "hashtable")) {
				char* tmp = cmd[2];
				int hash_idx = tmp[strlen(tmp) - 1] - '0';
				hash_init(&hash[hash_idx],hash_func, hash_less, NULL);
			}
			else if (!strcmp(cmd[1], "bitmap")) {
				char* tmp = cmd[2];
				int bitmap_idx = tmp[strlen(tmp) - 1] - '0';
				size_t size = atoi(cmd[3]);
				bitmap[bitmap_idx] = *bitmap_create(size);
			}
		}
		//dumpdata
		else if (!strcmp(cmd[0], "dumpdata")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			if (name[0] == 'l') {
				if (list_empty(&list[idx]))	
					continue;
				LIST_ELEM* curr = list_begin(&list[idx]);
				for (int i = 0; i < list_size(&list[idx]); i++) {
					LIST_ITEM* tmp = list_entry(curr, LIST_ITEM, elem);
					curr = list_next(curr);
					printf("%d ", tmp->data);
				}
				printf("\n");
			}
			else if (name[0] == 'h') {
				if (hash_empty(&hash[idx]))
					continue;
				HASH_ITERATER tmp;
				hash_first(&tmp, &hash[idx]);
				while (hash_next(&tmp)) {
					HASH_ITEM *item = hash_entry(hash_cur(&tmp), HASH_ITEM,elem);
					printf("%d ", item->data);
				}				
				printf("\n");
			}
			else if (name[0] == 'b') {
				if ((int)bitmap_size(&bitmap[idx]) == 0)
					continue;
				for (int i = 0; i < bitmap_size(&bitmap[idx]); i++) {
					if (bitmap_test(&bitmap[idx],i))
						printf("1");
					else 
						printf("0");
				}
				printf("\n");
			}
		}
		//delete
		else if (!strcmp(cmd[0], "delete")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			if (name[0] == 'l')
				list_delete(&list[idx]);
			else if (name[0] == 'h')
				hash_destroy(&hash[idx], hash_deleter);
			else if (name[0] == 'b')
				bitmap_destroy(&bitmap[idx]);
		}
		//list functions 
		//1. list_push_front, list_push_back
		else if (!strcmp(cmd[0], "list_push_front")) {
			char* name = cmd[1];
			int val = atoi(cmd[2]);
			int idx = name[strlen(name) - 1] - '0';
			LIST_ITEM* new = (LIST_ITEM*)malloc(sizeof(LIST_ITEM));
			new->data = val; 
			list_push_front(&list[idx], &(new->elem));
		}
		else if (!strcmp(cmd[0], "list_push_back")) {
			char* name = cmd[1];
			int val = atoi(cmd[2]);
			int idx = name[strlen(name) - 1] - '0';
			LIST_ITEM* new = (LIST_ITEM*)malloc(sizeof(LIST_ITEM));
			new->data = val;
			list_push_back(&list[idx], &(new->elem));
		}
		//2,. list_pop_front, list_pop_back
		else if (!strcmp(cmd[0], "list_pop_front")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			list_pop_front(&list[idx]);
		}
		else if (!strcmp(cmd[0], "list_pop_back")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			list_pop_back(&list[idx]);
		}
		//3. list_front, list_back
		else if (!strcmp(cmd[0], "list_front")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			LIST_ITEM* tmp = list_entry(list_front(&list[idx]), LIST_ITEM, elem);
			printf("%d\n", tmp->data);
		}
		else if (!strcmp(cmd[0], "list_back")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			LIST_ITEM* tmp = list_entry(list_back(&list[idx]), LIST_ITEM, elem);
			printf("%d\n", tmp->data);
		}
		//4, list_empty, list_size
		else if (!strcmp(cmd[0], "list_empty")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			if (list_empty(&list[idx]))
				printf("true\n");
			else
				printf("false\n");
		}
		else if (!strcmp(cmd[0], "list_size")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			printf("%zu\n", list_size(&list[idx]));
		}
		//5. list_max, list_min
		else if (!strcmp(cmd[0], "list_max")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			LIST_ITEM* tmp = list_entry(list_max(&list[idx], list_less, NULL), LIST_ITEM, elem);
			printf("%d\n", tmp->data);
		}
		else if (!strcmp(cmd[0], "list_min")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			LIST_ITEM* tmp = list_entry(list_min(&list[idx], list_less, NULL), LIST_ITEM, elem);
			printf("%d\n", tmp->data);
		}
		//6. list_insert, list_insert_ordered, list_remove
		else if (!strcmp(cmd[0], "list_insert")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			int insert_idx = atoi(cmd[2]); int val = atoi(cmd[3]);
			LIST_ELEM* curr = list_begin(&list[idx]);
			for (int i = 0; i < insert_idx; i++)
				curr = list_next(curr);
			LIST_ITEM* new = (LIST_ITEM*)malloc(sizeof(LIST_ITEM)); new->data = val;
			list_insert(curr, &(new->elem));
		}
		else if (!strcmp(cmd[0], "list_insert_ordered")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0'; int val = atoi(cmd[2]);
			LIST_ITEM* new = (LIST_ITEM*)malloc(sizeof(LIST_ITEM)); new->data = val;
			list_insert_ordered(&list[idx], &(new->elem), list_less, NULL);
		}
		else if (!strcmp(cmd[0], "list_remove")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			int remove_idx = atoi(cmd[2]);
			LIST_ELEM* curr = list_begin(&list[idx]);
			for (int i = 0; i < remove_idx; i++)
				curr = list_next(curr);
			list_remove(curr);
		}
		//7. list_unique
		else if (!strcmp(cmd[0], "list_unique")) {
			if (idx == 3) {
				char* name = cmd[1];
				char* dup_name = cmd[2];
				int idx = name[strlen(name) - 1] - '0';
				int dup_idx = dup_name[strlen(dup_name) - 1] - '0';
				list_unique(&list[idx], &list[dup_idx], list_less, NULL);
			}
			else {
				char* name = cmd[1];
				int idx = name[strlen(name) - 1] - '0';
				list_unique(&list[idx], NULL, list_less, NULL);
			}
		}
		// 8. list_reverse, list_sort
		else if (!strcmp(cmd[0], "list_reverse")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			list_reverse(&list[idx]);
		}
		else if (!strcmp(cmd[0], "list_sort")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			list_sort(&list[idx], list_less, NULL);
		}
		//9. list_splice
		else if (!strcmp(cmd[0], "list_splice")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			int before_idx = atoi(cmd[2]);
			char* insert_name = cmd[3]; 
			int insert_idx = insert_name[strlen(insert_name) - 1] - '0';
			int first_idx = atoi(cmd[4]); int last_idx = atoi(cmd[5]);
			LIST_ELEM* curr1 = (LIST_ELEM*)malloc(sizeof(LIST_ELEM));
			LIST_ELEM* curr2 = (LIST_ELEM*)malloc(sizeof(LIST_ELEM));
			LIST_ELEM* curr3 = (LIST_ELEM*)malloc(sizeof(LIST_ELEM));
			curr1 = list_begin(&list[idx]);
			curr2 = list_begin(&list[insert_idx]); curr3 = list_begin(&list[insert_idx]);
			for (int i = 0; i < before_idx; i++)
				curr1 = list_next(curr1);
			for (int j = 0; j < first_idx; j++)
				curr2 = list_next(curr2);
			for (int k = 0; k < last_idx; k++)
				curr3 = list_next(curr3);
			list_splice(curr1, curr2, curr3);
		}
		//10. list_swap, list_shuffle
		else if (!strcmp(cmd[0], "list_swap")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			int a_idx = atoi(cmd[2]); int b_idx = atoi(cmd[3]);
			LIST_ELEM* a_node = list_begin(&list[idx]);
			LIST_ELEM* b_node = list_begin(&list[idx]);
			for (int i = 0; i < a_idx; i++)
				a_node = list_next(a_node);
			for (int j = 0; j < b_idx; j++)
				b_node = list_next(b_node);

			/*if (a_idx > b_idx)
				list_swap(b_node, a_node);
			else*/
			list_swap(a_node, b_node);
		}
		else if (!strcmp(cmd[0], "list_shuffle")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			list_shuffle(&list[idx]);
		}
		//hash
		//1. hash_insert, hash_apply
		else if (!strcmp(cmd[0], "hash_insert")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			int insert_data = atoi(cmd[2]);
			HASH_ITEM *new = (HASH_ITEM*)malloc(sizeof(HASH_ITEM));
			new->data = insert_data;
			hash_insert(&hash[idx],&(new->elem));
		} 
		else if (!strcmp(cmd[0], "hash_apply")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			if (!strcmp(cmd[2], "square")) 
				hash_apply(&hash[idx], hash_square);
			else if (!strcmp(cmd[2], "triple"))
				hash_apply(&hash[idx], hash_triple);
		}
		//2. hash_empty, hash_size, hash_clear
		else if (!strcmp(cmd[0], "hash_empty")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			if (hash_empty(&hash[idx]))
				printf("true\n");
			else 
				printf("false\n");
		}
		else if (!strcmp(cmd[0], "hash_size")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			printf("%zu\n", hash_size(&hash[idx]));			
		}
		else if (!strcmp(cmd[0], "hash_clear")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			hash_clear(&hash[idx], hash_deleter);
		}
		//3. hash_delete
		else if (!strcmp(cmd[0], "hash_delete")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			int del_data = atoi(cmd[2]);
			HASH_ITEM *del_item = (HASH_ITEM*)malloc(sizeof(HASH_ITEM));
			del_item->data = del_data;
			hash_delete(&hash[idx], &(del_item->elem));
		}
		//4. hash_find
		else if (!strcmp(cmd[0], "hash_find")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			int find_data = atoi(cmd[2]);
			HASH_ITEM *find_item = (HASH_ITEM*)malloc(sizeof(HASH_ITEM));
			find_item->data = find_data;
			if (hash_find(&hash[idx], &(find_item->elem)) == NULL)
				continue;
			else 
				printf("%d\n", find_data);
		}
		//5. hash_replace
		else if (!strcmp(cmd[0], "hash_replace")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			int rep_data = atoi(cmd[2]);
			HASH_ITEM *rep_item = (HASH_ITEM*)malloc(sizeof(HASH_ITEM));
			rep_item->data = rep_data;
			hash_replace(&hash[idx], &(rep_item->elem));
		}

		//bitmap
		//1. bitmap_all, bitmap_mark
		else if (!strcmp(cmd[0],"bitmap_mark")) {
			char *name = cmd[1];
			int idx = name[strlen(name)-1] - '0';
			int mark_idx = atoi(cmd[2]); 
			bitmap_mark(&bitmap[idx], mark_idx);
		}
		else if (!strcmp(cmd[0], "bitmap_all")) {
			char *name = cmd[1];
			int idx = name[strlen(name)-1] - '0';
			int start = atoi(cmd[2]); int cnt = atoi(cmd[3]);
			if (bitmap_all(&bitmap[idx],start, cnt)) 
				printf("true\n");
			else
				printf("false\n");
		}
		//2. bitmap_any
		else if (!strcmp(cmd[0], "bitmap_any")) {
			char* name = cmd[1];
			int idx = name[strlen(name)-1] - '0';
			int start = atoi(cmd[2]); int cnt = atoi(cmd[3]);
			if (bitmap_any(&bitmap[idx], start, cnt))
				printf("true\n");
			else 
				printf("false\n");
		}
		//3. bitmap_contains
		else if (!strcmp(cmd[0], "bitmap_contains")) {
			char *name = cmd[1];
			int idx = name[strlen(name)-1] - '0';
			int start = atoi(cmd[2]); int cnt = atoi(cmd[3]);
			if (!strcmp(cmd[4], "true")) {
				if (bitmap_contains(&bitmap[idx], start, cnt, true))
					printf("true\n");
				else 
					printf("false\n");
			}
			else if (!strcmp(cmd[4], "false")) {
				if (bitmap_contains(&bitmap[idx], start, cnt, false))
					printf("true\n");
				else 
					printf("false\n");
			}
		}
		//4. bitmap_count
		else if (!strcmp(cmd[0], "bitmap_count")) {
			char *name = cmd[1];
			int idx = name[strlen(name)-1] - '0';
			int start = atoi(cmd[2]); int cnt = atoi(cmd[3]);
			if (!strcmp(cmd[4], "true")) {
				printf("%zu\n", bitmap_count(&bitmap[idx], start, cnt, true));
			}
			else if (!strcmp(cmd[4], "false")) {
				printf("%zu\n", bitmap_count(&bitmap[idx], start, cnt, false));
			}
		}
		//5. bitmap_dump
		else if (!strcmp(cmd[0],"bitmap_dump")) {
			char* name = cmd[1];
			int idx = name[strlen(name)-1] - '0';
			bitmap_dump(&bitmap[idx]);
		}
		//6. bitmap_flip
		else if (!strcmp(cmd[0], "bitmap_flip")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			int flip_idx = atoi(cmd[2]);
			bitmap_flip(&bitmap[idx], flip_idx);
		}
		//7. bitmap_none
		else if (!strcmp(cmd[0], "bitmap_none")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			int start = atoi(cmd[2]); int cnt = atoi(cmd[3]);
			if (bitmap_none(&bitmap[idx], start, cnt))
				printf("true\n");
			else 
				printf("false\n");
		}
		//8. bitmap_reset
		else if (!strcmp(cmd[0], "bitmap_reset")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			int reset_idx = atoi(cmd[2]);
			bitmap_reset(&bitmap[idx], reset_idx);
		}
		//9. bitmap_scan, bitmap_scan_and_flip
		else if (!strcmp(cmd[0], "bitmap_scan")) {
			char *name = cmd[1];
			int idx = name[strlen(name)-1] - '0';
			int start = atoi(cmd[2]); int cnt = atoi(cmd[3]);
			if (!strcmp(cmd[4], "true")) {
				printf("%zu\n", bitmap_scan(&bitmap[idx], start, cnt, true));
			}
			else if (!strcmp(cmd[4], "false")) {
				printf("%zu\n", bitmap_scan(&bitmap[idx], start, cnt, false));
			}
		}
		else if (!strcmp(cmd[0], "bitmap_scan_and_flip")) {
			char *name = cmd[1];
			int idx = name[strlen(name)-1] - '0';
			int start = atoi(cmd[2]); int cnt = atoi(cmd[3]);
			if (!strcmp(cmd[4], "true")) {
				printf("%zu\n", bitmap_scan_and_flip(&bitmap[idx], start, cnt, true));
			}
			else if (!strcmp(cmd[4], "false")) {
				printf("%zu\n", bitmap_scan_and_flip(&bitmap[idx], start, cnt, false));
			}
		}
		//10. bitmap_set, bitmap_set_all, bitmap_set_multiple
		else if (!strcmp(cmd[0], "bitmap_set")) {
			char *name = cmd[1];
			int idx = name[strlen(name)-1] - '0';
			int set_idx = atoi(cmd[2]);
			if (!strcmp(cmd[3], "true")) 
				bitmap_set(&bitmap[idx], set_idx, true);
			else if (!strcmp(cmd[3], "false")) 
				bitmap_set(&bitmap[idx], set_idx, false);
		}
		else if (!strcmp(cmd[0], "bitmap_set_all")) {
			char *name = cmd[1];
			int idx = name[strlen(name)-1] - '0';
			if (!strcmp(cmd[2], "true")) 
				bitmap_set_all(&bitmap[idx], true);
			else if (!strcmp(cmd[2], "false")) 
				bitmap_set_all(&bitmap[idx], false);
		}
		else if (!strcmp(cmd[0], "bitmap_set_multiple")) {
			char *name = cmd[1];
			int idx = name[strlen(name)-1] - '0';
			int start = atoi(cmd[2]); int cnt = atoi(cmd[3]);
			if (!strcmp(cmd[4], "true")) {
				bitmap_set_multiple(&bitmap[idx], start, cnt, true);
			}
			else if (!strcmp(cmd[4], "false")) {
				bitmap_set_multiple(&bitmap[idx], start, cnt, false);
			}
		}
		//11. bitmap_size
		else if (!strcmp(cmd[0], "bitmap_size")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			printf("%zu\n", bitmap_size(&bitmap[idx]));
		}
		//12. bitmap_test
		else if (!strcmp(cmd[0], "bitmap_test")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			int test_idx = atoi(cmd[2]);
			if (bitmap_test(&bitmap[idx], test_idx)) 
				printf("true\n");
			else 
				printf("false\n");
		}
		//13. bitmap_expand
		else if (!strcmp(cmd[0], "bitmap_expand")) {
			char* name = cmd[1];
			int idx = name[strlen(name) - 1] - '0';
			int size = atoi(cmd[2]);
			bitmap_expand(&bitmap[idx], size);
		}
	}
} 