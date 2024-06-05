#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_NUM_LENGTH 5
#define SIZEOF_CHAR 1

typedef struct {
    int rows;
    int cols;
    unsigned char *cells; 
} Map;

int get_num(FILE *file) {
    //skips white spaces and reads numeric value from file, returns value converted to integer
    //file must be non-null pointer to file opened for reading
    //returns EOF when end of file, -2 when error occurs


    //skip white spaces
    int c;
    while(isspace(c=fgetc(file))) {
    }

    char *buffer = calloc(MAX_NUM_LENGTH, SIZEOF_CHAR); //sizeof(char) == 1
    if (buffer == NULL) {
        fprintf(stderr, "Error: Unable to allocate memory!\n");
        return -2;
    }

    buffer[0] = c;
    int i;
    for (i = 0; i < MAX_NUM_LENGTH; i++) {
        if(!isdigit(c)){
            if(c == EOF) {
                return EOF;
            }
            else if(!isspace(c)) {
                fprintf(stderr, "Error: Only digits and white space characters are allowed in the file!\n");
                return -2;
            }
            break;
        }
        buffer[0] = (char)(c);
        c = fgetc(file);
    }

    if (i == MAX_NUM_LENGTH) {
        free(buffer);
        fprintf(stderr, "Error: Buffer overflow occured when reading from file!\n");
        return -2;
    }

    int n = (int)(strtol(buffer, NULL, 10)); //convert buffer to int value

    free(buffer);

    return n;
}

Map *init_map (int rows,int cols) {
    //returns a pointer to dynamically allocated structure of type Map or NULL if allocation is unsuccessful
    Map *map = calloc(1, sizeof(Map));
    if (map == NULL) {
        return NULL;
    }
    map->cells = calloc(rows*cols, SIZEOF_CHAR);
    if (map->cells == NULL) {
        free(map);
        return NULL;
    }
    return map;
}

Map *load_map(FILE *file) {
    //loads map structure - reads width, height and cells from file
    //file must be non-null pointer to file opened for reading
    //returns pointer to filled map or NULL
    int rows, cols;
    rows = get_num(file);
    cols = get_num(file);

    if (rows < 0 || cols < 0) {
        fprintf(stderr, "Error: rows and cols must be positive whole numbers!\n");
        return NULL;
    }
    Map *map = init_map(rows, cols);
    if (map == NULL) {
        fprintf(stderr, "Error: Unable to allocate memory!\n");
        return NULL;
    }
    map->rows = rows;
    map->cols = cols;
    
    int c;
    for(int i=0; i < rows*cols; i++) {
        c = get_num(file);
        if (c == EOF) {
            free(map);
            fprintf(stderr, "Error: Unexpected end of file when reading the map!\n");
            return NULL;
        }
        else if (c == -2) {
            free(map);
            return NULL;
        }
        map->cells[i] = (unsigned char)(c);
    }

    return map;
}

void free_map (Map *map) {
    free(map->cells);
    free(map);
    return;
}

bool isborder(Map *map, int r, int c, int border) {
    //border == 0 - left
    //border == 1 - right
    //border == 2 - upper/bottom
    //r,c are coordinates of the cell
    unsigned char cell_value = map->cells[(r-1) * (map->cols) + c - 1];

    if((cell_value >> border) & 1) { //bitwise shift combined with bitwise AND
        return true;
    }
    return false;
}

int start_border(Map *map, int r, int c, int leftright) {
    //return values:
    /*
    leva: 0
    prava: 1
    horni/spodni: 2 
    */

    if (leftright > 0) { //pravidlo prave ruky    
        if (r % 2 && (c == 1)) {//lichy radek, vstup zleva
            return 1;
        }
        if (!(r % 2) && (c == 1)) {//vstup zleva na sudem radku
            return 2;
        }
        if (c == map->cols) {//vstup zprava
            if (r%2) {//ma horni hranici - lichy radek ma vzdy horni hranici
                return 2;
            }
            else { //ma spodni hranici - sudy radek
                return 0;
            }
        }
        if (r == 1) {//vstup zhora
            return 0;
        }
        if (r == map->rows) {//vstup zezdola
            return 1;
        }
    }
    else { //pravidlo leve ruky
        if (r % 2 && (c == 1)) {//lichy radek, vstup zleva
            return 2;
        }
        if (!(r % 2) && (c == 1)) {//vstup zleva na sudem radku
            return 1;
        }
        if (c == map->cols) {//vstup zprava
            if (r % 2) {//ma horni hranici
                return 0;
            }
            else {  //ma spodni hranici
                return 2;
            }
        }
        if (r==1) { //vstup zhora
            return 1;
        }
        if (r==map->rows) {//vstup zezdola
            return 0;
        }
    }
    return -1;
}

bool is_valid_maze(Map *map) {
    for (int i = 0; i < (((map->rows) * (map->cols))-1); i++) {
        if (map->cells[i] > 7) {
            return false;
        }

        if (map->cells[i] & (1)) {                  //has left border
            if (i % map->cols) {                    //is not on left side of map
                if (!(map->cells[i-1] & (1<<1))) {  //neighbour on left doesn't have right border
                    return false;
                }
            }
        }
        if (map->cells[i] & (1<<1)) {               //has right border
            if ((i+1) % map->cols) {                  //is not on right side of map
                if (!(map->cells[i+1] & (1)))       //neighbour on right doesn't have left border
                return false;
            }
        }
        if (map->cells[i] & (1<<2)) {                                   //has upper/bottom border
            if (i >= map->cols && i < ((map->rows - 1) * map->cols)) {   //is not on top/bottom of the map
                if (!(i%2)) {                                              //cell is in V shape
                    if (!((map->cells[i - map->cols]) & (1 << 2))) {    //neighbour on top doesn't have bottom border
                        return false;
                    }
                }
                else {                                                  //cell is in A shape
                    if(!((map->cells[i + map->cols]) & (1 << 2))) {     //neighbour on bottom doesn't have upper border
                        return false;
                    }
                }
            }

        }
    }
    return true;
}

int next_edge(int previous_edge, int orientation, int direction) {
    //returns next preffered edge in given direction (-1 = left, 1 = right) of a triangle entered via previous_edge
    //orientation: -1 = V, 1 = A
    if(orientation > 0) { //A shape
        if (direction < 0) { //turn left
            return (previous_edge + 1 + 3) % 3;
        }
        else { //turn right
            return (previous_edge - 1 + 3) % 3;
        }
    }
    else { // V shape
        if (direction < 0) {
            //turn left
            return (previous_edge - 1 + 3) % 3;
        }
        else {
            //turn right
            return (previous_edge + 1 + 3) % 3;
        }
    }
}

void print_path(Map *map, int r, int c, unsigned char border_crossed, int turn) {
    /*
        define transition matrix:
        first index is for triangle orientation: 0 down, 1 up
        second index is number of edge that we will cross
        third index is used to look up:
            change of the row number
            change of column number
            edge number we just crossed, using edge numbering of new triangle
    */
    int transition_matrix[2][3][3] = {
        {//downwards oriented triangle
            {0, -1, 1}, //edge 0 - left
            {0,  1, 0}, //edge 1 - right
            {-1, 0, 2}  //edge 2 - up
        },
        {//upwards oriented triangle
            {0, -1, 1}, //edge 0 - left
            {0,  1, 0}, //edge 1 - right
            {1,  0, 2}  //edge 2 - down
        }
    };

    int edge, *move;

    while(true) {
        //return when end of maze is reached
        if (r < 1 || r > map->rows || c < 1 || c > map->cols) {
            return;
        }

        //edge is border that was crossed on the way in the current triangle
        edge = border_crossed;

        //print coordinates of current triangle
        printf("%d,%d\n", r, c);

        while (true) {
            //gets preffered edge to cross
            edge = next_edge(edge, ((r+c)%2) ? (1) : (-1), turn);

            //checks if it is possible to cross edge
            if(((map->cells[(r-1)*(map->cols) + c - 1] >> edge) & (1)) == 0) {
                //if true, continue to next triangle via the edge
                break;
            }
        }

        //get move from transition matrix
        move = transition_matrix[(r+c)%2][edge];
        //update cell position and the edge number
        r += move[0];
        c += move[1];
        border_crossed = move[2];
        
    }
}

unsigned char entering_border(Map *map, int c) {
    //returns border that is passed when entering the maze
    if (c==1) {
        return 0;
    }
    else if (c==map->cols) {
        return 1;
    }
    else {
        return 2;
    }
}

void print_help() {
    printf("\
    Maze manual:\n\
    Usage:\n\
    maze <argument> [R] [C] [file]\n\
    Arguments:\n\
    --rpath\tFinds a way in a maze in file by only turning right, starting in row R and column C.\n\
    --lpath\tFinds a way in a maze in file by only turning left, starting in row R and column C.\n\
    --test\tChecks if a file is valid maze.\n\
    --shortest\tDoes nothing at all.\n\
    ");
}

int main (int argc, char **argv) {

    FILE *file = NULL;
    Map *map;
    int leftright;
    int r,c;


    if (strcmp("--help", argv[1]) == 0) {
        print_help();
        return 0;
    }

    if (strcmp("--test", argv[1]) == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error, file missing!\n");
            return 1;
        }

        file = fopen(argv[2], "r");
        if (file == NULL) {
            fprintf(stderr, "Error opening file!\n");
            return 1;
        }
        map = load_map(file);
        if(map == NULL) {
            return 1;
        }
        if(!is_valid_maze(map)) {
            printf("Invalid\n");
        }
        else {
            printf("Valid\n");
        }
        
        free_map(map);
        fclose(file);
        return 0;
    }
    else if (strcmp("--rpath", argv[1]) == 0) {
        if (argc < 5) {
            fprintf(stderr, "Invalid arguments, see ./maze --help for help.\n");
        }
        r = strtoul(argv[2],NULL,10);
        c = strtoul(argv[3],NULL,10);
        leftright = 1;
    }
    else if (strcmp("--lpath", argv[1]) == 0) {
        if (argc < 5) {
            fprintf(stderr, "Invalid arguments, see ./maze --help for help.\n");
        }
        r = strtoul(argv[2],NULL,10);
        c = strtoul(argv[3],NULL,10);
        leftright = -1;
    }

    file = fopen(argv[4], "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file!\n");
        return 1;
    }

    //read all cells to a structure
    map = load_map(file);
    if (map == NULL) {
        return 1;
    }

    if(!is_valid_maze(map)) {
        fprintf(stderr, "Error: File is not a valid maze!\n");
        return 1;
    }

   
    //find and prints path
    print_path(map, r, c, entering_border(map, c), leftright);

    //close file and free allocated memory
    fclose(file);
    free_map(map);
}
