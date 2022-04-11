#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "keyinput.h"

#define MAX_NB_MENU_ENTRIES 10
typedef struct tagmenuEntry {
    char entry_key;
    char* entry_description;
} menuEntry_t;

typedef enum {
    APP_MENU_HOME = 0,
    APP_MENU_1,
    APP_MENU_2,
    NB_APP_MENU,
    APP_MENU_NONE
} appMenu_t;

typedef enum {
    APP_CMD_UNKNOWN = 0,
    APP_CMD_1,
    APP_CMD_2,
    APP_CMD_EXIT,
    APP_CMD_MAX
} appCmd_t;

typedef struct {
  char* FuncName;
  int (*Func)(void* in, void* out);
} appAction_t;

bool doRun = true;
bool requireInput = false;
bool requireConfirm = false;
appMenu_t AppMenu = APP_MENU_HOME;

menuEntry_t appMenu[NB_APP_MENU][MAX_NB_MENU_ENTRIES] = {
    [APP_MENU_HOME] =   {   {'1', (char*)"Entry 1 of menu 1"},
                            {'2', (char*)"Entry 2 of menu 1"},
                            {'X', (char*)"Exit menu 1"}},
    [APP_MENU_1] =      {   {'1', (char*)"Entry 1 of menu 2"},
                            {'2', (char*)"Entry 2 of menu 2"},
                            {'X', (char*)"Back"}},
    [APP_MENU_2] =      {   {'1', (char*)"Entry 1 of menu 3"},
                            {'2', (char*)"Entry 2 of menu 3"},
                            {'X', (char*)"Back"}},
};

int function1(void* in, void* out) {
  fprintf(stdout, "\rfunction 1: in = %f\n", in ? *((float*)in) : 0.0);
  *((float*)out) = 100.0;
  fprintf(stdout, "\rfunction 1 executed\n");
  return 0;
}

int function2(void* in, void* out) {
  fprintf(stdout, "\rfunction 2 executed\n");
  return 0;
}

appAction_t menuFuncs[] = {
    [APP_CMD_UNKNOWN]   = {(char*)"Unknown", NULL},
    [APP_CMD_1]         = {(char*)"Function1", function1},
    [APP_CMD_2]         = {(char*)"Function2", function2},
};

int appExit(appMenu_t currAppMenu) {
  if (currAppMenu) return 0;
  fprintf(stdout, "\r");
  exit(0);
  return 0;
}

static void printMenu (menuEntry_t menu[]) {
    if (!menu) return;
    for (int i = 0; i < MAX_NB_MENU_ENTRIES; i++) {
        if (menu[i].entry_description)
            fprintf(stdout, "\r[%c]. %s.\n", menu[i].entry_key, menu[i].entry_description);
    }
    fprintf(stdout, "\r");
}

static bool requestUserConfirm (){ 
    char confirmLetter[128] = {0};
    while (strcmp(confirmLetter,"Y") && strcmp(confirmLetter,"y") && 
           strcmp(confirmLetter,"N") && strcmp(confirmLetter,"n")) {
        fprintf(stdout, "\r[Y]yes to confirm | [N]no to cancel\n\r");
        memset(confirmLetter, 0, sizeof(confirmLetter));
        scanf("%s", confirmLetter);
        fprintf(stdout, "\rconfirmLetter = %s\n", confirmLetter);
    }
    if (!strcmp(confirmLetter,"Y") || !strcmp(confirmLetter,"y")) {
        fprintf(stdout, "\ruser confirmed\n");
        return true;
    }
    fprintf(stdout, "\ruser canceled\n");
    return false;
}

/**
 * @brief parseUserCommand
 * @param ch 
 * @param currAppMenu 
 * @return appCmd_t 
 */
static appCmd_t parseUserCommand (char ch, appMenu_t currAppMenu)
{
    appCmd_t currUserCmd = APP_CMD_UNKNOWN;
    if (ch == 'x' || ch == 'X') appExit(currAppMenu);
    switch (currAppMenu)
    {
        case APP_MENU_HOME:
            if (ch == '1')
                currUserCmd = APP_CMD_1;
            else if (ch == '2')
                currUserCmd = APP_CMD_2;
            else if (ch == 'x' || ch == 'X')
              currUserCmd = APP_CMD_EXIT;
            if (currUserCmd != APP_CMD_EXIT)
                requireInput = true;
            break;

        case APP_MENU_1:
            if (ch == '1')
                currUserCmd = APP_CMD_2;
            else if (ch == 'x' || ch == 'X')
              currUserCmd = APP_CMD_EXIT;
            if (currUserCmd != APP_CMD_EXIT)
            break;

        default:
            break;
    }
    return currUserCmd;
}


/**
 * @brief showMenu
 * @param currAppMenu 
 */
static void showMenu (appMenu_t currAppMenu)
{
    switch (currAppMenu)
    {
        case APP_MENU_HOME:
            system("clear");
            printMenu(appMenu[APP_MENU_HOME]);
            break;

        case APP_MENU_1:
            system("clear");
            printMenu(appMenu[APP_MENU_1]);
            break;

        case APP_MENU_2:
            system("clear");
            printMenu(appMenu[APP_MENU_2]);
            break;

        default:
            break;
    }
}

/**
 * @brief readUserInput
 * @param currAppMenu 
 * @param currUserCmd 
 * @param input_value 
 * @param doConfirm 
 * @return int 
 */
static int readUserInput (appMenu_t currAppMenu, appCmd_t currUserCmd, float *input_value, bool doConfirm)
{
    fprintf(stdout, "\ruser input: \n");
    scanf("%f", input_value);
    if (currUserCmd != APP_CMD_UNKNOWN) requireConfirm = true;
    if (doConfirm)
        if (!requestUserConfirm())
            return -EXIT_FAILURE;
    return EXIT_SUCCESS;
}

/**
 * @brief processUserCommand
 * @param currUserCmd 
 * @param currUserParam 
 * @return appMenu_t 
 */
appMenu_t processUserCommand(appCmd_t currUserCmd, float currUserParam)
{
    appMenu_t nextAppMenu = APP_MENU_HOME;
    if (currUserCmd >= APP_CMD_MAX) {
      fprintf(stdout, "\rInvalid user command\n");
      return nextAppMenu;
    }
    float result = 0.0;
    if (menuFuncs[currUserCmd].Func) menuFuncs[currUserCmd].Func(&currUserParam, &result);
    fprintf(stdout, "\rmenuFuncs[%d] = %s, param = %f, result = %f\n", currUserCmd, menuFuncs[currUserCmd].FuncName, currUserParam, result);
    return nextAppMenu;
}

void console_ui ()
{
    int ch = 0;
    appMenu_t currAppMenu = APP_MENU_HOME;
    showMenu(currAppMenu);
    while (ch != 'x' && ch != 'X') {
        //update menu
        usleep(5e5);
        showMenu(currAppMenu);
        
        //read keyboard
        ch = keyinput_kbhit (); 
        if (ch != 0) printf("\rkey pressed == %c\n", (unsigned char)ch);
        else continue;

        //process user keyboard hit
        appCmd_t currUserCmd = parseUserCommand (ch, currAppMenu);
        if (currUserCmd == APP_CMD_UNKNOWN || currUserCmd >= APP_CMD_MAX)
          continue;

        //request command param from user + confirmation
        float currUserParam = 0.0;
        if (readUserInput(currAppMenu, currUserCmd, &currUserParam, requireConfirm)) continue;
        fprintf(stdout, "\ruser input = %f\n", currUserParam);

        //process user command with input param
        currAppMenu = processUserCommand(currUserCmd, currUserParam);

        //delay
        usleep(2e6);
    }
    doRun = false;
}

int main ()
{
    //init custom terminal mode
    if (!keyinput_init()) {
        printf("\rFailed to init keyread\n");
        return -EXIT_FAILURE;
    }

    //run console UI
    console_ui();

    appExit((appMenu_t)0);
    return 0;
}
