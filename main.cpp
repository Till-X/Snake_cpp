#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include <ncurses.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>


#define H 22
#define W 22
#define GAME_OVER 0
#define GAME_BEGIN 1


char qp[H][W];
int zb[2][100];
int X1,Y1;
long start;
bool check_eaten;
int grade,score,gamespeed,timeover;
char dir;
int tail_x, tail_y;
bool game;

int fd=0;
int flags;
char buf[10];

typedef struct {
    int x[100], y[100], len, state;
}Snake;

Snake snake;
void food();
void show_once(int grade,int score,int gamespeed);
void snake_init();
int kbhit(void);


void chessboard_init(){
    int i,j;
    for(i=1;i<=H-2;i++)
        for(j=1;j<=W-2;j++)
            qp[i][j]=' ';
    for(i=0;i<=H-1;i++)
        qp[0][i] = qp[H-1][i] = '#';
    for(i=1;i<=H-2;i++)
        qp[i][0] = qp[i][W-1] = '#';
}

void food(){
    srand(time(0));
    do
    {
        X1=rand()%W-2+1;
        Y1=rand()%H-2+1;
    }
    while((qp[X1][Y1]!=' ')&&(X1!=tail_x && Y1!=tail_y));
    qp[X1][Y1]='$';
}

void snake_init(){
    snake.len = 4;
    snake.state = 'w';
    snake.x[1] = H/2;
    snake.y[1] = W/2;
    qp[snake.x[1]][snake.y[1]]='@';
    tail_x=snake.x[snake.len];
    tail_y=snake.y[snake.len];
    for(int i=2;i<=snake.len;i++){
        snake.x[i] = snake.x[i-1] + 1;
        snake.y[i] = snake.y[i-1];
        qp[snake.x[i]][snake.y[i]]='*';
    }
}

void draw_snake(){
    qp[tail_x][tail_y]=' ';
    qp[snake.x[1]][snake.y[1]]='@';
    for(int i=2;i<=snake.len;i++){
        qp[snake.x[i]][snake.y[i]]='*';
    }
}

void show_once(int grade,int score,int gamespeed){
    int i,j;
    system("clear");
    for(i=0;i<H;i++)
    {
        printf("\t");
        for(j=0;j<W;j++)
            printf("%c ",qp[i][j]);
        if(i==0) printf( "\tGrade: %d",grade);
        if(i==2) printf( "\tScore: %d",score);
        if(i==4) printf( "\tAutomatic forward");
        if(i==5) printf( "\ttime interval: %d ms" ,gamespeed);
        printf("\n");
    }
}

void game_init(){
    chessboard_init();
    snake_init();
    food();
}

void check_foodeat(){
    if(snake.x[1] == X1 && snake.y[1] == Y1){
        score=score+100;
        gamespeed=gamespeed-20000;
        check_eaten = 1;
        snake.len++;
        food();
        
    }
}

bool check_snakelive(){
    if(snake.x[1]==0 || snake.x[1]== H-1 || snake.y[1]==0 || snake.y[1]== W-1){//撞墙
        // printf("\tGame over!\n"); 
        return 0;
    }
    for(int i=2; i<=snake.len;i++){
        if(snake.x[i]==snake.x[1] && snake.y[i]==snake.y[1]){
            // printf("\tGame over!\n");
            return 0;
        }   
    } 
    return 1;
}


void snake_move() {
    if(!check_eaten){
        tail_x=snake.x[snake.len];
        tail_y=snake.y[snake.len];
        for(int i=snake.len; i>1;i--){
            snake.x[i]=snake.x[i-1];
            snake.y[i]=snake.y[i-1];
        }
    }
    else {
        for(int i=snake.len; i>1;i--){
            snake.x[i]=snake.x[i-1];
            snake.y[i]=snake.y[i-1];
        }
    }
    switch(dir){
        case 'w':
            if(snake.state=='s')
                snake.x[1]++;
            else
                snake.x[1]--,snake.state = 'w';
            break;
        case 's': 
            if(snake.state=='w')
                snake.x[1]--;
            else
                snake.x[1]++,snake.state = 's';
            break;
        case 'a':
            if(snake.state=='d')
                snake.y[1]++;
            else
                snake.y[1]--,snake.state = 'a';
            break;
        case 'd':
            if(snake.state=='a')
                snake.y[1]--;
            else
                snake.y[1]++,snake.state = 'd';
            break;
        default:
            if(snake.state=='s')snake.x[1]++;
            else if(snake.state=='w')snake.x[1]--;
            else if(snake.state=='d')snake.y[1]++;
            else if(snake.state=='a')snake.y[1]--;
            break;
    }
    check_eaten = 0;
    draw_snake();
}

void game_run(){
    snake_move();
    if(check_snakelive()==0){
        game = GAME_OVER;
    }
    check_foodeat();
}

void key_check(){
    char ch;
    if(kbhit()){
        dir = getchar();
    }
    switch(ch){
        case 'w':dir='w';break;
        case 's':dir='s';break;
        case 'a':dir='a';break;
        case 'd':dir='d';break;
    }
}


int main(void)
{
    score=0;
    gamespeed=500000;
    game=GAME_BEGIN;
    game_init();
    show_once(1,score,gamespeed);
    start = clock();
    while(game){
        key_check();
        if(clock()-start>=gamespeed){
            game_run();
            show_once(1,score,gamespeed/1000);
            start = clock();
        }
    }
    printf("\tGame over!\n"); 

}

int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if(ch != EOF){
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}