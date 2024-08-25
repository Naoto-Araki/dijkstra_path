#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NODE_NUM 10 /* 総ノード数 */
#define MAX 9999 /* 無限大に相当する数 */
#define FLAG 1 /* Dijkstraのテストの場合は0に、シミュレーション評価を行う場合は1にする */
#define HISTORY_SIZE 100000

//構造体を定義する
typedef struct {
    int src; // 送信元ノード
    int dest; // 受信先ノード
    int success;
    int sum_success;
} CommunicationHistory;

int main(){
/* Dijkstraのアルゴリズム部分で必要な変数 */
  int graph[NODE_NUM][NODE_NUM]; /* 距離行列 */ 
  int path[NODE_NUM]; /* 前ノード表 */
  int dist[NODE_NUM]; /* 距離を格納 */
  int chk[NODE_NUM]; /* 最短距離確定のフラグ */
  int tmp_node, tmp_dist; /* 注目しているノードとそこまでの最短距離 */
  int src, dest; /* 始点・終点ノード */
  int a, b, c, d, i, j;
  int k, l = -1;
  int fin; /* 未確定ノードが残っているかどうかのフラグ */
  FILE *fp; /* 距離の入ったファイルを示すポインタ */

  /* シミュレーション評価の部分で必要な変数 */
  int link[NODE_NUM][NODE_NUM]; /* リンク容量 */
  int bandwidth[NODE_NUM][NODE_NUM]; /* リンクの空き容量 */
  int success; /* 確立できた通信回数 */
  int sum_success; /* 確立できた通信回数の合計 */
  int sim_time; /* 評価の回数をカウント */

  CommunicationHistory history[HISTORY_SIZE]; // 通信の履歴を格納する配列
  int historyIndex = 0; // 通信履歴のインデックス
  int n = 100;
  int total_times = 10000;

  /*距離行列の作成*/
  for(i=0; i<NODE_NUM; i++){
    for(j=0; j<NODE_NUM; j++){
      graph[i][j] = MAX; /* 接続されていないノード間の距離をMAXにする */
      link[i][j] = -1; /* 接続されていないノード間のリンク容量を-1にする */
      if(i==j){graph[i][j] = 0; link[i][j] = -1;}/* そのノード自身への距離は0とし、リンク容量は-1とする */
    }
  }
  fp=fopen("./distance.txt", "r");
  while(fscanf(fp, "%d %d %d %d", &a, &b, &c, &d) != EOF) /* EOFまで4つ組を読み込む */
    {
      graph[a][b]=c; /* 接続されているノード間の距離を設定 */
      graph[b][a]=c; /* 逆方向も等距離と仮定 */
      link[a][b]=d; /* 接続されているノード間のリンクを設定 */
      link[b][a]=d; /* 逆方向も同じ容量と仮定 */
    }
  fclose(fp);

/* 
始点・終点ノードを標準入力から得る (評価の場合は、実行しない)
*/
  if (FLAG == 0){
    printf("Source Node?(0-%d)", NODE_NUM-1);
    fscanf(stdin, "%d", &src);
    printf("Destination Node?(0-%d)", NODE_NUM-1);
    fscanf(stdin, "%d", &dest);
  }

  if (FLAG == 1) srand((unsigned)time(NULL)); /* 乱数の初期化, これ以降、rand()で乱数を得ることができる */

  for (i=0; i<NODE_NUM; i++){ /* 全リンクの空き容量を初期状態に戻す */
    for (j=0; j<NODE_NUM; j++){
      bandwidth[i][j] = link[i][j];   //ここで容量がリセットされている
    }
  }

  /*シミュレーション開始*/
  success = 0; sum_success = 0; /* 評価指標を初期化 */
  for (sim_time=0; sim_time<total_times; sim_time++){
    if (FLAG == 1){
        /* ランダムに送受信ノードを決定 */
        src = rand() % NODE_NUM;
        dest = rand() % NODE_NUM;
        printf("src=%d, dest=%d\n", src, dest); /* 送受信ノードを表示 */
        if (src==dest) printf("送受信ノードが一致している\n");
    }

    /* dijkstraアルゴリズム */
    /*初期化*/
    for(i=0; i<NODE_NUM; i++){ /* 何も確定していない状態にする */
        dist[i] = MAX;
        chk[i] = 0;
        path[i] = NODE_NUM;
    }
    path[src] = src; /* 始点ノードへの経路上の前ノードはそれ自身とする */
    dist[src] = 0; /* 始点ノード自身への距離は0である */
    chk[src] = 1; /* 始点ノードへの最短距離は確定 */
    tmp_node = src; /* 始点ノードから探索を始める */
    fin = 0;

    /* 経路探索*/
    for(i=0; i<NODE_NUM; i++){
        if (graph[src][i] != MAX) {
            dist[i] = graph[src][i];
            path[i] = src;
        }
    }

    for (i = 0; i < NODE_NUM; i++) {
        if (src == i){
            chk[src] = 1;
            tmp_node = src;
            tmp_dist = dist[i];
        }else if(path[i] == src){
            if(k == -1 || dist[i] < dist[k] || (dist[i] == dist[k] && i < k))
            k = i;
        }
    }

    if(k != -1){
        chk[k] = 1;
        tmp_node = k;
        tmp_dist = dist[k];
    }

    while (fin == 0) {
        for (i = 0; i < NODE_NUM; i++) {
          if(graph[tmp_node][i] == 1){
            if(tmp_dist + graph[tmp_node][i] < dist[i]){
              dist[i] = tmp_dist + graph[tmp_node][i]; 
              path[i] = tmp_node;
            }
          }
        }
        
        int l = -1;
        for (i = 0; i < NODE_NUM; i++) {
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

    if(FLAG == 0){
        if(dist[dest]>=MAX){
            printf("No path from node%d to node%d.\n",src,dest);
        }else{
            printf("Shortest path from node%d to node%d is as follows.\n",src,dest);
            printf("%d <- ",dest);
            i=dest;
            for(i=path[i]; i!=src; i=path[i]){
                printf("%d <- ",i);
            }
            printf("%d\n",src);
            printf("Shortest distance is %d.\n", dist[dest]);
        }
        return 0; 
    }

    int allBandwidthPositive = 1;
    for (i = dest; i != src; i = path[i]) { /* 前ノード表を辿る */
      int link_src = path[i];
      int link_dest = i;
      /*その経路上の全てのリンクに空き容量がある場合、それらを1Mbpsだけ減少させ、「確立できた通信回数」を1増やす*/
      if (bandwidth[link_src][link_dest] != -1 && bandwidth[link_src][link_dest] > 0){
        bandwidth[link_src][link_dest] -= 1;
        bandwidth[link_dest][link_src] -= 1;
      }else{
        allBandwidthPositive = 0;
      }
    }
    if (allBandwidthPositive) {
      success = sum_success;
      sum_success += 1;
      history[historyIndex].src = src;
      history[historyIndex].dest = dest;
      history[historyIndex].success = success;
      history[historyIndex].sum_success = sum_success;
    }

    historyIndex = (historyIndex + 1) % HISTORY_SIZE;

  if (n != 0 && success != 0) {
      // n回前の成功がある場合
      int nBackIndex = ((historyIndex - 1) - n ) % HISTORY_SIZE; // 負の結果を防ぐためにHISTORY_SIZEを加える
      int nBack_dest = history[nBackIndex].dest;
      int nBack_src = history[nBackIndex].src;

      if (history[nBackIndex].sum_success - history[nBackIndex].success == 1){
        for(i=0; i<NODE_NUM; i++){ /* 何も確定していない状態にする */
          dist[i] = MAX;
          chk[i] = 0;
          path[i] = NODE_NUM;
        }
        path[nBack_src] = nBack_src; /* 始点ノードへの経路上の前ノードはそれ自身とする */
        dist[nBack_src] = 0; /* 始点ノード自身への距離は0である */
        chk[nBack_src] = 1; /* 始点ノードへの最短距離は確定 */
        tmp_node = nBack_src; /* 始点ノードから探索を始める */
        fin = 0;

        /* 経路探索*/
        for(i=0; i<NODE_NUM; i++){
            if (graph[nBack_src][i] != MAX) {
                dist[i] = graph[nBack_src][i];
                path[i] = nBack_src;
            }
        }

        for (i = 0; i < NODE_NUM; i++) {
            if (nBack_src == i){
                chk[nBack_src] = 1;
                tmp_node = nBack_src;
                tmp_dist = dist[i];
            }else if(path[i] == nBack_src){
                if(k == -1 || dist[i] < dist[k] || (dist[i] == dist[k] && i < k))
                k = i;
            }
        }

        if(k != -1){
            chk[k] = 1;
            tmp_node = k;
            tmp_dist = dist[k];
        }

        while (fin == 0) {
            for (i = 0; i < NODE_NUM; i++) {
              if(graph[tmp_node][i] == 1){
                if(tmp_dist + graph[tmp_node][i] < dist[i]){
                  dist[i] = tmp_dist + graph[tmp_node][i]; 
                  path[i] = tmp_node;
                }
              }
            }
            
            int l = -1;
            for (i = 0; i < NODE_NUM; i++) {
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

              if (chk[nBack_dest] == 1) {
                  fin = 1;
              }
        }

        for (i = nBack_dest; i != nBack_src; i = path[i]) { /* 前ノード表を辿る */
          int link_src = path[i];
          int link_dest = i;
          /*その経路上の全てのリンクに空き容量がある場合、それらを1Mbpsだけ減少させ、「確立できた通信回数」を1増やす*/
            bandwidth[link_src][link_dest] += 1;
            bandwidth[link_dest][link_src] += 1;
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
