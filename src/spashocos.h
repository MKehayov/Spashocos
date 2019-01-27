#define BOARD_WIDTH 51
#define BOARD_LENGTH 20
#define AMOUNT_ENEMIES 40
#define POWER_UP_SHOTS 50

#define USLEEP_DEFAULT 200
#define GAME_DEFAULT_SPEED 50
#define SHOOTING_SPEED 1700

#define POINTS_SHOT 10
#define POINTS_SMALL_SHIP 50
#define POINTS_MED_SHIP 100
#define POINTS_BIG_SHIP 150
#define POINTS_SELF_DESTR -100

#define SHOT_TYPE_OWN 1
#define SHOT_TYPE_ENEMY 0

#define ENEMY_ID_1 0
#define ENEMY_ID_2 1
#define ENEMY_ID_3 2
#define OWN_SHIP 3
#define EMPTY_SPACE 4
#define ENEMY_SHOT 5
#define OWN_SHOT 6
#define UPGRADE 7

extern int life;
extern char board[BOARD_LENGTH][BOARD_WIDTH];
extern int curr_enemies;
extern int power_up_shots;
extern WINDOW* game_window;
extern char** args;
extern pthread_t own_ship_tid;
extern int points;

typedef struct {
  void* param1_;
  void* param2_;
}parameters;

typedef struct {
  short x_;
  short y_;
}position;



void* shot(void* params);
void* enemies();

void* upgradePlacer();
void pointsManager(int type);
void printBoard();
void* ship();

//utils.c
int randomEnemySpawnPosition();
int randomEnemyMovement();
int randomEnemyType();
position randomUpgradePosition();
void initBoard();
void gameOver();



