#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NODE_NUM 10
#define MAX 9999
#define HISTORY_SIZE 10000

// 通信の履歴を格納する構造体
typedef struct {
    int src; // 送信元ノード
    int dest; // 受信先ノード
    int success;
    int sum_success;
    int max_path[NODE_NUM];
} CommunicationHistory;

int graph[NODE_NUM][NODE_NUM]; /* 距離行列 */
int path[NODE_NUM];            /* 前ノード表 */
int dist[NODE_NUM];            /* 距離を格納 */
int chk[NODE_NUM];             /* 最短距離確定のフラグ */
int tmp_node, tmp_dist;         /* 注目しているノードとそこまでの最短距離 */
int src, dest;                 /* 始点・終点ノード */
int a, b, c, d, i, j;
int k, l = -1;
int fin; /* 未確定ノードが残っているかどうかのフラグ */

/* シミュレーション評価の部分で必要な変数 */
int link[NODE_NUM][NODE_NUM]; /* リンク容量 */
int bandwidth[NODE_NUM][NODE_NUM]; /* リンクの空き容量 */
int success; /* 確立できた通信回数 */
int sum_success; /* 確立できた通信回数の合計 */
int sim_time; /* 評価の回数をカウント */
int g = 0;

CommunicationHistory history[HISTORY_SIZE]; // 通信の履歴を格納する配列
int historyIndex = 0; // 通信履歴のインデックス
int n = 1;
int total_times = 10000;

// グラフのリンク構造体
struct Link{
    int src;
    int dest;
    int weight;
};

// グラフのリンクリスト構造体
struct LinkList{
    struct Link link;
    struct LinkList *next;
};

// グラフのリンクを追加
void addLink(struct LinkList **head, struct Link link){
    struct LinkList *newLink = (struct LinkList *)malloc(sizeof(struct LinkList));
    newLink->link = link;
    newLink->next = *head;
    *head = newLink;
}

// リンクを重みの大きい順にソート
void sortLinks(struct LinkList **head){
    struct LinkList *sortedLinks = NULL;
    struct LinkList *current = *head;

    while (current != NULL){
        struct LinkList *next = current->next;
        if (sortedLinks == NULL || current->link.weight >= sortedLinks->link.weight){
            current->next = sortedLinks;
            sortedLinks = current;
        }else{
            struct LinkList *currentSorted = sortedLinks;
            while (currentSorted->next != NULL && currentSorted->next->link.weight > current->link.weight){
                currentSorted = currentSorted->next;
            }
            current->next = currentSorted->next;
            currentSorted->next = current;
        }
        current = next;
    }

    *head = sortedLinks; // ソート済みリストに置き換え
}

// DFSによる経路探索
int dfs(int node, int dest, int visited[], int gPrime[NODE_NUM][NODE_NUM], int parent[]){
    if (node == dest){
        return 1; // ノード dest に到達した場合、経路が存在する
    }
    visited[node] = 1;

    for (int i = 0; i < NODE_NUM; i++){
        if (gPrime[node][i] != MAX && !visited[i]){
            parent[i] = node; // 親ノードを記録
            if (dfs(i, dest, visited, gPrime, parent)){
                return 1;
            }
        }
    }
    return 0; // ノード dest に到達しなかった場合、経路は存在しない
}

void printMaxPath(int src, int dest, int parent[], int max_path[NODE_NUM]){
    if (dest == src){
        printf("%d", src);
        return;
    }
    printMaxPath(src, parent[dest], parent, max_path);
    printf(" -> %d", dest);
    max_path[dest] = parent[dest];
}

void increaselink(int src, int dest, int parent[], int max_path[NODE_NUM], int bandwidth[NODE_NUM][NODE_NUM]){
    if (dest == src){
        //printf("%d", src);
        return;
    }
    increaselink(src, parent[dest], parent, max_path, bandwidth);
    //printf(" -> %d", dest);

    // リンク容量を+1
    int link_src = parent[dest];
    int link_dest = dest;
    bandwidth[link_src][link_dest] += 1;
    bandwidth[link_dest][link_src] += 1;

    // 現在のリンク容量を表示
    //printf(" - Link (%d, %d) Capacity: %d\n", link_src, link_dest, bandwidth[link_src][link_dest]);
}



// Dijkstraのアルゴリズムにより最短経路を計算
void dijkstra(int src, int dest, int gPrime[NODE_NUM][NODE_NUM], int dist[NODE_NUM], int path[NODE_NUM]){
    int chk[NODE_NUM] = {0};
    fin = 0;

    for (int i = 0; i < NODE_NUM; i++){
        dist[i] = MAX;
        chk[i] = 0;
        path[i] = NODE_NUM;
    }

    path[src] = src;
    dist[src] = 0;
    chk[src] = 1;
    int tmp_node = src;
    int tmp_dist = 0;

    for(int i = 0; i < NODE_NUM; i++){
        if (gPrime[src][i] != MAX) {
            dist[i] = gPrime[src][i];
            path[i] = src;
        }
    }

    for (int i = 0; i < NODE_NUM; i++) {
        if (src == i){
            chk[src] = 1;
            tmp_node = src;
            tmp_dist = dist[i];
        } else if (path[i] == src){
            if(tmp_dist + gPrime[src][i] < dist[i]){
                dist[i] = tmp_dist + gPrime[src][i];
                path[i] = tmp_node;
            }
        }
    }

    int k = -1;
    for (int i = 0; i < NODE_NUM; i++) {
        if (chk[i] == 0) {
            if(k == -1 || dist[i] < dist[k] || (dist[i] == dist[k] && i < k)){
                k = i;
            }
        }
    }

    if(k != -1){
        chk[k] = 1;
        tmp_node = k;
        tmp_dist = dist[k];
    }

    while (fin == 0) {
        for (int i = 0; i < NODE_NUM; i++) {
            if(gPrime[tmp_node][i] != MAX){
                if(tmp_dist + gPrime[tmp_node][i] < dist[i]){
                    dist[i] = tmp_dist + gPrime[tmp_node][i];
                    path[i] = tmp_node;
                }
            }
        }

        int l = -1;
        for (int i = 0; i < NODE_NUM; i++) {
            if (chk[i] == 0) {
                if(l == -1 || dist[i] < dist[l] || (dist[i] == dist[l] && i < l)){
                    l = i;
                }
            }
        }

        if(l != -1){
            chk[l] = 1;
            tmp_node = l;
            tmp_dist = dist[l];
        }

        if (chk[dest] == 1) {
            fin = 1;
        }
    }
}

int main(){
    int graph[NODE_NUM][NODE_NUM];
    struct LinkList *links = NULL;

    FILE *fp = fopen("./distance.txt", "r");
    int a, b, c, d;
    while (fscanf(fp, "%d %d %d %d", &a, &b, &c, &d) != EOF)
    {
        graph[a][b] = c;
        graph[b][a] = c;
        link[a][b] = d;
        link[b][a] = d;
        if (a != b)
        {
            struct Link link = {a, b, d};
            addLink(&links, link);
        }
    }
    fclose(fp);

    srand((unsigned)time(NULL));
    
    for (i = 0; i < NODE_NUM; i++){ /* 全リンクの空き容量を初期状態に戻す */
        for (j = 0; j < NODE_NUM; j++){
            bandwidth[i][j] = link[i][j]; // ここで容量がリセットされている
        }
    }

    struct LinkList *current = links;

    // リンクを重みの大きい順にソート
    sortLinks(&links);

    success = 0;
    sum_success = 0; /* 評価指標を初期化 */
    for (sim_time = 0; sim_time < total_times; sim_time++){
        int src, dest;
        /* ランダムに送受信ノードを決定 */
        src = rand() % NODE_NUM;
        dest = rand() % NODE_NUM;

        printf("src=%d, dest=%d\n", src, dest); /* 送受信ノードを表示 */
        if (src == dest)
            printf("送受信ノードが一致している\n");

        int visited[NODE_NUM];
        for (int i = 0; i < NODE_NUM; i++){
            visited[i] = 0;
        }

        int gPrime[NODE_NUM][NODE_NUM];
        for (int i = 0; i < NODE_NUM; i++){
            for (int j = 0; j < NODE_NUM; j++){
                gPrime[i][j] = MAX;
                link[i][j] = -1; /* 接続されていないノード間のリンク容量を-1にする */
                if (i == j){
                    graph[i][j] = 0;
                    link[i][j] = -1;
                }
            }
        }

        int parent[NODE_NUM];
        int linkAdded[NODE_NUM][NODE_NUM];
        memset(linkAdded, 0, sizeof(linkAdded));

        while (1){
            // DFSにより src から dest への経路が存在するかどうかを調べる
            for (int i = 0; i < NODE_NUM; i++){
                visited[i] = 0; // 訪問状態をリセット
            }
            if (dfs(src, dest, visited, gPrime, parent)){
                printf("Maximum Shortest Path from node %d to node %d in G': ", src, dest);

                // Calculate the shortest path using Dijkstra's algorithm
                dijkstra(src, dest, gPrime, dist ,parent);

                // Print the shortest path
                printMaxPath(src, dest, parent, history[historyIndex].max_path);
                printf("\n");

                int allBandwidthPositive = 1;
                for (i = dest; i != src; i = parent[i]) { /* 前ノード表を辿る */
                int link_src = parent[i];
                int link_dest = i;
                /*その経路上の全てのリンクに空き容量がある場合、それらを1Mbpsだけ減少させ、「確立できた通信回数」を1増やす*/
                if (bandwidth[link_src][link_dest] != -1 && bandwidth[link_src][link_dest] > 0){
                    //printf("容量先%d\n", bandwidth[link_src][link_dest]);
                    //printf("link_src%d\n", link_src);
                    //printf("link_dest%d\n", link_dest);
                    bandwidth[link_src][link_dest] -= 1;
                    bandwidth[link_dest][link_src] -= 1;
                    //printf("容量後%d\n", bandwidth[link_src][link_dest]);
                }else{
                    allBandwidthPositive = 0;
                }
                }
                // 通信が確立している場合
                if (allBandwidthPositive){
                    success = sum_success;
                    sum_success += 1;
                    history[historyIndex].src = src;
                    history[historyIndex].dest = dest;
                    history[historyIndex].success = success;
                    history[historyIndex].sum_success = sum_success;
                    //printMaxPath(src, dest, parent, history[historyIndex].max_path);
                    //historyIndex = (historyIndex + 1) % HISTORY_SIZE;
                    //printf("%d\n", sum_success);
                }
                historyIndex = (historyIndex + 1) % HISTORY_SIZE;

                // 最初の通信成功が n 回前の成功で、n 回前に発生した通信が確立している場合
                if (n != 0 && success != 0){
                    // n 回前の成功がある場合
                    int nBackIndex = ((historyIndex - 1) - n) % HISTORY_SIZE; // 負の結果を防ぐためにHISTORY_SIZEを加える
                    int nBack_dest = history[nBackIndex].dest;
                    int nBack_src = history[nBackIndex].src;

                    // n 回前の通信が確立している場合                 
                    if (history[nBackIndex].sum_success - history[nBackIndex].success == 1){
                        // 通信が確立している経路上の空き容量を1Mbpsだけ増加させる
                        increaselink(nBack_src, nBack_dest, history[nBackIndex].max_path, history[nBackIndex].max_path, bandwidth);
                    }
                }
                break; // 経路が見つかったのでループを終了
            }else{
                //printf("No path from node %d to node %d in G'.\n", src, dest);
                // 経路が存在しない場合、次に最大の b(e) をもつリンクを G' に追加する
                int maxWeight = -1;
                current = links;
                while (current != NULL){
                    if (current->link.weight > maxWeight && !linkAdded[current->link.src][current->link.dest]){
                        maxWeight = current->link.weight;
                    }
                    current = current->next;
                }

                if (maxWeight == -1){
                    //printf("No more links to add in G'.\n");
                    break;
                }

                current = links;
                while (current != NULL){
                    if (current->link.weight == maxWeight && !linkAdded[current->link.src][current->link.dest]){
                        int srcLink = current->link.src;
                        int destLink = current->link.dest;
                        gPrime[srcLink][destLink] = current->link.weight;
                        gPrime[destLink][srcLink] = current->link.weight;
                        linkAdded[srcLink][destLink] = 1;
                        linkAdded[destLink][srcLink] = 1;
                    }
                    current = current->next;
                }
            }
        }
    }

    printf("成功回数は%d\n", sum_success);
    printf("呼損回数は");
    printf("%d\n", total_times - sum_success); 
    //printf("呼損率は %d%%\n", (total_times - sum_success) * 100 / total_times);
    printf("呼損率は %.1f%%\n", ((float)(total_times - sum_success) * 100.0) / total_times);
    return 0;


}