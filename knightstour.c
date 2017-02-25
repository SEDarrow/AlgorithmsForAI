#include <stdlib.h>
#include <stdio.h>

int* array;     // For the generated tour
int* degrees;   // For the number of open spots that can be moved to from each spot


struct point {
    int x;
    int y;
    int d;
};


/*
 * Modes for choosing the next move
 * RANDOM: choose the next move moving clockwise
 * DISTANCE: choose the next move based on its distance from the center of the board
 * DEGREE: choose the next move with the lowest degree
 */
enum moveMethod {
    RANDOM = 0,
    DISTANCE = 1,
    DEGREE = 2
};



/*
 * Fill array with zeros
 * Size is the number of rows in a square 2D array
 */
void initializeArray(int size) {
    int i;
    for(i = 0; i < size*size; i++) {
        array[i] = 0;
    }
}


/*
 * Set the degree of each square on the board before any moves are made
 * Each square will have a dregree of possible moves between 2 (in the corners) and 8 (max # of moves)
 */
void initializeDegrees(int size) {
    int i, j;
    for(i = 0; i < size; i++) {
      for(j = 0; j < size; j++) {
                degrees[i*size +j] = 8;
                if(i < 2 || i >= size-2) degrees[i*size +j] -=2;
                if(j < 2 || j >= size-2) degrees[i*size +j] -=2;
                if(i < 1 || i >= size-1) {
                   if(j < 2 || j >= size-2) degrees[i*size +j] -= 1;
                   else degrees[i*size +j] -= 2;
                }
                if(j < 1 || j >= size-1) {
                   if(i < 2 || i >= size-2) degrees[i*size +j] -= 1;
                   else degrees[i*size +j] -= 2;
                }
        }
    }
}



/*
 * Generates possible moves sorted in order of decreasing degree
 * The sortedMoves parameter will be filled with the possible moves
 * The number of valid moves in sortedMoves will be returned
 */
int getMovesByDegree(int size, struct point start, struct point* sortedMoves, int spotNum) {
    int x, y, m, i, j;

    // Possible changes to the start point to create moves
    struct point moves[] = {{2, -1}, {2, 1}, {1, -2}, {1, 2}, {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}};
    int numMoves = 0;

    for(m = 0; m < 8; m++) {

        // calculate possible move
        moves[m].x += start.x;
        moves[m].y += start.y;

        x = moves[m].x;
        y = moves[m].y;

        if(x < size && x >= 0 && y < size && y >= 0) { // if move is on board

            // Update degrees to acknowledge move to start spot
            degrees[y*size +x]--;

            if(array[y*size+x] == 0) { // if square hasn't been visited
                numMoves++;
                moves[m].d = degrees[y*size +x];
            }
            else // Weigh invalid move so it won't be put in array of generated moves
                moves[m].d = 9;
        }
        else // Weigh invalid move so it won't be put in array of generated moves
            moves[m].d = 9;
    }

    int min;
    struct point temp;

    // sort moves in increasing degree
    for(i = 0; i < numMoves; i++) {
        min = i;
        for(j = i+1; j < 8; j++) {
            if(moves[j].d < moves[min].d) min = j;
        }

        sortedMoves[i] = moves[min];

        temp = moves[i];
        moves[i] = moves[min];
        moves[min] = temp;
    }

    return numMoves;
}


/*
 * Generates possible moves sorted in order of increasing distance from the center of the board
 * The sortedMoves parameter will be filled with the possible moves
 * The number of valid moves in sortedMoves will be returned
 */
int getMovesByDistance(int size, struct point start, struct point* sortedMoves) {
    int x, y, m, i, j;

    // Possible changes to the start point to create moves
    struct point moves[] = {{2, -1}, {2, 1}, {1, -2}, {1, 2}, {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}};
    int center = size/2;
    int numMoves = 0;

    // Get distance of each possible moves from the center
    for(m = 0; m < 8; m++) {

        //calculate possible move
        moves[m].x += start.x;
        moves[m].y += start.y;

        x = moves[m].x;
        y = moves[m].y;
        if(x < size && x >= 0 && y < size && y >= 0 && array[y*size+x] == 0) { // if spot is valid and open
            numMoves++;
            moves[m].d = abs(x-center) + abs(y-center);
        }
        else moves[m].d = -1; // Weigh invalid move so it won't be in the generated moves array
    }

    int max;
    struct point temp;

    // Sort moves in decreasing distance from center
    for(i = 0; i < numMoves; i++) {
        max = i;
        for(j = i+1; j < 8; j++) {
            if(moves[j].d > moves[max].d) max = j;
        }

        sortedMoves[i] = moves[max];

        temp = moves[i];
        moves[i] = moves[max];
        moves[max] = temp;
    }

    return numMoves;
}


/*
 * Generates a knight's tour recursively
 * Fills the global array variable with the generated tour
 * Returns number of spots visited in generating the tour
 */
long findTour(int size, struct point start, int spotNum, long visited, int moveMethod) {
    array[start.y*size+start.x] = spotNum;

    // Return a negative identifier if tour is complete
    if(spotNum == size*size) return visited*-1;

    struct point moves[8];
    int numMoves, x, y, m;
    int xChange[8] = {-2, -1, 1, 2, 2, 1, -1, -2};
    int yChange[8] = {1, 2, 2, 1, -1, -2, -2, -1};

    // Get possible next moves
    switch(moveMethod) {
        case RANDOM:
            for(m = 0; m < 8; m++) {
                x = start.x + xChange[m];
                y = start.y + yChange[m];
                if(x < size && x >= 0 && y < size && y >= 0 && array[y*size+x] == 0) {
                    visited = findTour(size, (struct point){x, y}, spotNum+1, visited+1, moveMethod);
                    if(visited < 0) return visited;
                }
            }
            array[start.y*size+start.x] = 0;
            return visited;
        case DISTANCE:
            numMoves = getMovesByDistance(size, start, moves);
            break;
        case DEGREE:
            numMoves = getMovesByDegree(size, start, moves, spotNum);
            break;
    }

    for(m = 0; m < numMoves; m++) {
        visited = findTour(size, moves[m], spotNum+1, visited+1, moveMethod);
        if(visited < 0) return visited;
    }

    array[start.y*size+start.x] = 0;
    return visited;
}

int main() {
   int size;
   for(size = 5; size < 9; size++) {
       array = (int*)malloc(sizeof(int)*size*size);
       degrees = (int*)malloc(sizeof(int)*size*size);

       initializeArray(size);
       initializeDegrees(size);
       printf("Size: %d Method: DEGREE Visited: %ld\n", size, -1*findTour(size, (struct point){0, 0}, 1, 1, DEGREE));

       initializeArray(size);
       printf("Size: %d Method: DISTANCE Visited: %ld\n", size, -1*findTour(size, (struct point){0, 0}, 1, 1, DISTANCE));

       initializeArray(size);
       printf("Size: %d Method: RANDOM Visited: %ld\n", size, -1*findTour(size, (struct point){0, 0}, 1, 1, RANDOM));

       free(array);
       free(degrees);
   }
}


