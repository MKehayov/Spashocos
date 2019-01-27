#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include "spashocos.h"

int points = 0;
int life = 0;
int power_up_shots = 0;
int missed = 0;
int curr_enemies;
char board[BOARD_LENGTH][BOARD_WIDTH];
pthread_t own_ship_tid;
position your_ship = {BOARD_WIDTH/2, BOARD_LENGTH - 2};
WINDOW *game_window;
char **args;

char my_ship[4] = {'U','U', 'U', '\0'};
char enemy[3][6] = {{'O','O','O','\0'},
                    {'O','O','O','O','\0'},
                    {'O','O','O','O','O','\0'}};


void pointsManager(int type)
{
  switch(type) {

   case ENEMY_ID_1  :
      points += POINTS_SMALL_SHIP;
      break; /* optional */
	
   case ENEMY_ID_2  :
      points += POINTS_MED_SHIP;
      break; /* optional */

    case ENEMY_ID_3  :
      points += POINTS_BIG_SHIP;
      break; /* optional */

    case ENEMY_SHOT  :
      points +=POINTS_SHOT;
      break; /* optional */

    case OWN_SHIP  :
      points += POINTS_SELF_DESTR;
      break; /* optional */
  
   default : /* Optional */
    break;
  }
}

void printBoard()
{
  werase(game_window);
  for(int y = 0; y < BOARD_LENGTH; ++y)
  {
    for(int x = 0; x < BOARD_WIDTH; ++x)
    {
      if(board[y][x] != EMPTY_SPACE)
      {
        if(board[y][x] <= ENEMY_ID_3)
          mvwprintw(game_window, y,x, enemy[(short)board[y][x]]);
        else if(board[y][x] == OWN_SHIP)
          mvwprintw(game_window, y,x, my_ship);
        else if(board[y][x] == ENEMY_SHOT)
          mvwprintw(game_window, y,x, "v");
        else if(board[y][x] == OWN_SHOT)
          mvwprintw(game_window, y,x, ".");
        else if(board[y][x] == UPGRADE)
          mvwprintw(game_window, y,x, "X");
      }
    }
  }

  mvwprintw(game_window, 1,1, "IAIK-RAGEQUITTER");
  mvwprintw(game_window, 1,43, "v.1.1.0");
  mvwprintw(game_window, 2,34, "Points: %08d", points);
  mvwprintw(game_window, 2,11, "Missed: %d", missed);
  mvwprintw(game_window, 2,1, "Life: %d", life);
  mvwprintw(game_window, 2,22, "Shots:%04d", power_up_shots);

  box(game_window, 0,0);
  wrefresh(game_window);
}

void* ship()
{
  int ch;
  board[your_ship.y_][your_ship.x_] = (char)OWN_SHIP;

  while(1)
  {
    if(life <= 0)
      return 0;
    ch = getchar();

    if(ch == 'a' /*check for character and boundaries*/ && (your_ship.x_ - 1) > 0)
    {
      board[your_ship.y_][your_ship.x_] = (char)EMPTY_SPACE;
      your_ship.x_--;
      board[your_ship.y_][your_ship.x_] = (char)OWN_SHIP;
    }
    else if(ch == 'w' /*check for character  and boundaries*/ && (your_ship.y_ - 1) > 2)
    {
      board[your_ship.y_][your_ship.x_] = (char)EMPTY_SPACE;
      your_ship.y_--;
      board[your_ship.y_][your_ship.x_] = (char)OWN_SHIP;
    }
    else if(ch == 's' /*check for character  and boundaries*/ && (your_ship.y_ + 1) < (BOARD_LENGTH - 1))
    {
      board[your_ship.y_][your_ship.x_] = (char)EMPTY_SPACE;
      your_ship.y_++;
      board[your_ship.y_][your_ship.x_] = (char)OWN_SHIP;
    }
    else if(ch == 'd' /*check for character  and boundaries*/ && (your_ship.x_ + 2) < (BOARD_WIDTH - 2))
    {
      board[your_ship.y_][your_ship.x_] = (char)EMPTY_SPACE;
      your_ship.x_++;
      board[your_ship.y_][your_ship.x_] = (char)OWN_SHIP;
    }
    else if(ch == ' ' /*check for character*/)
    {
      if(board[your_ship.y_ - 1][your_ship.x_] == OWN_SHOT ||
          board[your_ship.y_ - 1][your_ship.x_ + 1] == OWN_SHOT ||
          board[your_ship.y_ - 1][your_ship.x_ + 2] == OWN_SHOT)
        continue;

      if(power_up_shots > 0)
      {
        pthread_t shot_thread_1;
        pthread_t shot_thread_2;

        position shot_pos_1 = {your_ship.x_, your_ship.y_ - 1};
        parameters shot_params_1 = {(void*)&shot_pos_1, (void*)SHOT_TYPE_OWN};

        position shot_pos_2 = {your_ship.x_ + 2, your_ship.y_ - 1};
        parameters shot_params_2 = {(void*)&shot_pos_2, (void*)SHOT_TYPE_OWN};

        pthread_create(&shot_thread_1, NULL, shot, (void*)&shot_params_1);
        pthread_create(&shot_thread_2, NULL, shot, (void*)&shot_params_2);

        power_up_shots--;
      }
      else
      {
        pthread_t shot_thread;

        position shot_pos = {your_ship.x_ + 1, your_ship.y_ - 1};
        parameters shot_params = {(void*)&shot_pos, (void*)SHOT_TYPE_OWN};

        pthread_create(&shot_thread, NULL, shot, (void*)&shot_params);
      }
    }

    ch = 0;
  }
}

void* shot(void* params)
{
  parameters* params_struct = (parameters*) params;
  position* pos = params_struct->param1_;

  int x = (int)pos->x_;
  int y = (int)pos->y_;
  
  size_t type = (size_t)params_struct->param2_;

  int time_count = 0;
  usleep(USLEEP_DEFAULT);
  if(!type)
    board[y][x] = (char)ENEMY_SHOT;
  else
    board[y][x] = (char)OWN_SHOT;

  do
  {
    if((board[y][x] == ENEMY_SHOT && type == 1) || (board[y][x] == OWN_SHOT && type == 0) || board[y][x] == EMPTY_SPACE)
    {
      if(type)
        pointsManager(ENEMY_SHOT);
      return 0;
    }

    usleep(900);
    time_count++;
    if((time_count) == 190)
    {
      board[y][x] = EMPTY_SPACE;
      if(!type)
      {
        if(board[y + 1][x] == OWN_SHOT)
        {
          board[y + 1][x] = (char)EMPTY_SPACE;
          return 0;
        }
        if(((x) > 0 && board[y + 1][x] <= OWN_SHIP) ||
        ((x - 1) > 0 && board[y + 1][x - 1] <= OWN_SHIP) ||
        ((x - 2) > 0 && board[y + 1][x - 2] <= OWN_SHIP) ||
        ((x - 3) > 0 && board[y + 1][x - 3] == ENEMY_ID_2) ||
        ((x - 4) > 0 && board[y + 1][x - 4] == ENEMY_ID_3))
        {
          if((board[y + 1][x - 2] == OWN_SHIP) || (board[y + 1][x - 1] == OWN_SHIP) || (board[y + 1][x] == OWN_SHIP))
            life--;

          return 0;
        }
        //for better collission detection
        if(((x) > 0 && board[y][x] <= 3) ||
          ((x - 1) > 0 && board[y][x - 1] <= OWN_SHIP) ||
          ((x - 2) > 0 && board[y][x - 2] <= OWN_SHIP) ||
          ((x - 3) > 0 && board[y][x - 3] == ENEMY_ID_2) ||
          ((x - 4) > 0 && board[y][x - 4] == ENEMY_ID_3))
        {
          if((board[y][x - 2] == OWN_SHIP) || (board[y][x - 1] == OWN_SHIP) || (board[y][x] == OWN_SHIP))
            life--;

          return 0;
        }
        y++;
        board[y][x] = ENEMY_SHOT;
      }
      else if(type)
      {
        if(board[y - 1][x] == ENEMY_SHOT)
        {
          board[y - 1][x] = (char)EMPTY_SPACE;
            return 0;
        }

        if((x) > 0 && board[y - 1][x] <= OWN_SHIP)
        {
          if(board[y - 1][x] == OWN_SHIP)
          {
            life--;
          }
          pointsManager(board[y - 1][x]);
          board[y - 1][x] = (char)EMPTY_SPACE;
          return 0;
        }
        else if((x - 1) > 0 && board[y - 1][x - 1] <= OWN_SHIP)
        {
          if(board[y - 1][x - 1] == (char)OWN_SHIP)
          {
            life--;
          }
          pointsManager(board[y - 1][x - 1]);
          board[y - 1][x - 1] = (char)EMPTY_SPACE;
          return 0;
        }
        else if((x - 2) > 0 && board[y - 1][x - 2] <= OWN_SHIP)
        {
          if(board[y - 1][x - 2] == OWN_SHIP)
          {
            life--;
          }
          pointsManager(board[y - 1][x - 2]);
          board[y - 1][x - 2] = (char)EMPTY_SPACE;
          return 0;
        }
        else if((x - 3) > 0 && board[y - 1][x - 3] != 0 && board[y - 1][x - 3] <= ENEMY_ID_3)
        {
          pointsManager(board[y - 1][x - 3]);
          board[y - 1][x - 3] = (char)EMPTY_SPACE;
          return 0;
        }
        else if((x - 4) > 0 && board[y - 1][x - 4] <= ENEMY_ID_3)
        {
          pointsManager(board[y - 1][x - 4]);
          board[y - 1][x - 4] = (char)EMPTY_SPACE;
          return 0;
        }
        //***********For much better Collission-Detection*******************
        else if((x) > 0 && board[y - 2][x] <= OWN_SHIP)
        {
          if(board[y - 2][x] == OWN_SHIP)
          {
            life--;
          }
          pointsManager(board[y - 2][x]);
          board[y - 2][x] = EMPTY_SPACE;
          return 0;
        }
        else if((x - 1) > 0 && board[y][x - 1] <= OWN_SHIP)
        {
          if(board[y][x - 1] == OWN_SHIP)
          {
            life--;
          }
          pointsManager(board[y][x - 1]);
          board[y][x - 1] = (char)EMPTY_SPACE;
          return 0;
        }
        else if((x - 2) > 0 && board[y][x - 2] <= OWN_SHIP)
        {
          if(board[y][x - 2] == OWN_SHIP)
          {
            life--;
          }
          pointsManager(board[y][x - 2]);
          board[y][x - 2] = (char)EMPTY_SPACE;
          return 0;
        }
        else if((x - 3) > 0 && board[y][x - 3] != 0 && board[y][x - 3] <= ENEMY_ID_3)
        {
          pointsManager(board[y][x - 3]);
          board[y][x - 3] = (char)EMPTY_SPACE;
          return 0;
        }
        else if((x - 4) > 0 && board[y][x - 4] <= ENEMY_ID_3)
        {
          pointsManager(board[y][x - 4]);
          board[y][x - 4] = (char)EMPTY_SPACE;
          return 0;
        }

        //***********************************************************

        y--;
        board[y][x] = OWN_SHOT;
      }

      time_count = 0;
    }



    if(y <= 2 || y >= (BOARD_LENGTH -1))
    {
      board[y][x] = EMPTY_SPACE;
      return 0;
    }
  } while(life > 0);

  return 0;
}

void* enemies()
{
  //needed variables
  int time_count = 0;
  int random_num = 0;
  //Mod 3 for 3 types of enemies
  int type_of_enemy = randomEnemyType();

  int x = randomEnemySpawnPosition();
  short y = 3;

  //x-coordinate + (length - 1) enemy and remaining left border (1)
  if(x == 0)
    x++;

  board[y][x] = type_of_enemy;

  do
  {
    usleep(2000);

    time_count++;

    if((board[y][x] == EMPTY_SPACE || (board[y][x] == OWN_SHOT)))
    {
      curr_enemies++;
      return 0;
    }

    //Collission-detection for your ship against enemies
    if((board[y][x] == OWN_SHIP ||
      board[y][x - 1] == OWN_SHIP ||
      board[y][x - 2] == OWN_SHIP ||
      board[y][x + 1] == OWN_SHIP ||
      board[y][x + 2] == OWN_SHIP) ||
      (type_of_enemy == ENEMY_ID_2 && board[y][x + 3] == OWN_SHIP) ||
      (type_of_enemy == ENEMY_ID_3 && (board[y][x + 3] == OWN_SHIP || board[y][x + 4] == OWN_SHIP)))

    {
      life = 0;
    }

    if((time_count)%500 == 0)
    {
      random_num = randomEnemyMovement();
      if(random_num == 0 && ((x - 1) > 1))
      {
        board[y][x] = (char)EMPTY_SPACE;
        x--;
        board[y][x] = (char)type_of_enemy;
      }
      else if(random_num == 1 && ((x + type_of_enemy + 3) < (BOARD_WIDTH - 1)))
      {
        board[y][x] = (char)EMPTY_SPACE;
        x++;
        board[y][x] = (char)type_of_enemy;
      }
    }
    if((time_count)%SHOOTING_SPEED == 0)
    {
      if(type_of_enemy == ENEMY_ID_1)
      {
        pthread_t enemy_shot_thread;

        position enemy_shot_pos = {x + 1, y + 2};
        parameters enemy_shot_params = {(void*)&enemy_shot_pos, (void*)SHOT_TYPE_ENEMY};

        pthread_create(&enemy_shot_thread, NULL, shot, (void*)&enemy_shot_params);
      }
      else if(type_of_enemy == ENEMY_ID_2)
      {
        pthread_t enemy_shot_thread_1;
        pthread_t enemy_shot_thread_2;

        position enemy_shot_pos_1 = {x, y + 2};
        position enemy_shot_pos_2 = {x + 3, y + 2};
        parameters enemy_shot_params_1 = {(void*)&enemy_shot_pos_1, (void*)SHOT_TYPE_ENEMY};
        parameters enemy_shot_params_2 = {(void*)&enemy_shot_pos_2, (void*)SHOT_TYPE_ENEMY};

        pthread_create(&enemy_shot_thread_1, NULL, shot, (void*)&enemy_shot_params_1);
        pthread_create(&enemy_shot_thread_2, NULL, shot, (void*)&enemy_shot_params_2);
      }
      else if(type_of_enemy == ENEMY_ID_3)
      {
        pthread_t enemy_shot_thread_1;
        pthread_t enemy_shot_thread_2;
        pthread_t enemy_shot_thread_3;

        position enemy_shot_pos_1 = {x, y + 2};
        position enemy_shot_pos_2 = {x + 2, y + 2};
        position enemy_shot_pos_3 = {x + 4, y + 2};
        parameters enemy_shot_params_1 = {(void*)&enemy_shot_pos_1, (void*)SHOT_TYPE_ENEMY};
        parameters enemy_shot_params_2 = {(void*)&enemy_shot_pos_2, (void*)SHOT_TYPE_ENEMY};
        parameters enemy_shot_params_3 = {(void*)&enemy_shot_pos_3, (void*)SHOT_TYPE_ENEMY};

        pthread_create(&enemy_shot_thread_1, NULL, shot, (void*)&enemy_shot_params_1);
        pthread_create(&enemy_shot_thread_2, NULL, shot, (void*)&enemy_shot_params_2);
        pthread_create(&enemy_shot_thread_3, NULL, shot, (void*)&enemy_shot_params_3);
      }
      time_count = 0;
    }

    if((time_count)%300 == 0)
    {
      if(y == (BOARD_LENGTH - 1))
      {
        missed++;
        return 0;

      }

      board[y][x] = (char)EMPTY_SPACE;
      y++;
      board[y][x] = (char)type_of_enemy;
    }


  } while(life > 0);

  return 0;
}

void* upgradePlacer()
{
  position pos = randomUpgradePosition();
  int y = pos.y_;
  int x = pos.x_;
  board[y][x] = UPGRADE;
  size_t count = 0;
  do
  {
    usleep(USLEEP_DEFAULT*2);
    count++;

    if(board[y][x] == OWN_SHIP || board[y][x - 1] == OWN_SHIP || board[y][x - 2] == OWN_SHIP)
    {
      power_up_shots++;

      if(board[y][x] == OWN_SHIP)
        board[y][x] = OWN_SHIP;
      else
        board[y][x] = EMPTY_SPACE;
      return 0;
    }
    else if(board[y][x] != UPGRADE)
    {
      board[x][y] = EMPTY_SPACE;
      return 0;
    }
    else if((count%10000) == 0)
    {
      board[y][x] = (char)EMPTY_SPACE;
      return 0;
    }

  } while(life > 0);

  return 0;
}

int main(int argc, char *argv[])
{
  args = argv;

  initBoard();
  void* return_value_join = 0;

  //Init own Ship
  pthread_create(&own_ship_tid, NULL, ship, NULL);

  //Timer variable
  size_t count = 0;

  do
  {
    usleep(GAME_DEFAULT_SPEED);
    count++;    

    if((count%8500) == 0 && curr_enemies > 0)
    {
      pthread_t enemy_thread;

      pthread_create(&enemy_thread, NULL, enemies, NULL);
      
      curr_enemies--;
    }
    
    if(missed == AMOUNT_ENEMIES)
      life = 0;
    
    
    if((count%50000) == 0)
    {
      pthread_t upgrade_thread;

      pthread_create(&upgrade_thread, NULL, upgradePlacer, NULL);
    }


    printBoard();

  } while(life > 0);

  //Preparing for termination
  pthread_cancel(own_ship_tid);
  
  gameOver();	
  
  pthread_join(own_ship_tid, return_value_join);

  if(return_value_join == PTHREAD_CANCELED)
    return 0;

  return -1;

  return 3;
}

