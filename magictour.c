#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int* array;     // For the generated tour
int* degrees;   // For the number of open spots that can be moved to from each spot
int* totals;    // Row and column totals: Index 0-7 for rows, 8-15 for columns
int* filled;    // Number of filled spots in each row and column: Index 0-7 for rows, 8-15 for columns


struct point {
    int x;
    int y;
    int d;
};


/*
 * Modes for choosing the next move
 * DEGREE: choose the next move with the lowest degree
 * BOUNDS: choose the next move to widen the bounds
 */
enum method {
    DEGREE = 0,
    BOUNDS = 1
};


/*
 * Prints array in matrix form
 * Size is number of rows
 */
void printArray(int size) {
    int i, j;
    for(i = 0; i < size; i++) {
      for(j = 0; j < size; j++) {
                printf("%d\t", array[i*size +j]);
        }
        printf("\n");
    }
    printf("\n");
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
 * Set the degrees of squares that have set moves
 * This function will only be used when moves are pre-loaded at the beginning of tour construction
 * Otherwise, degrees are updated as possible moves are considered
 */
void updateDegrees(int size) {
    struct point moves[] = {{2, -1}, {2, 1}, {1, -2}, {1, 2}, {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}};
    int x, y, i, j, m;
    for(i = 0; i < size; i++) {
        for(j = 0; j < size; j++) {
            if(array[j*size+i] == 0) continue;
            for(m = 0; m < 8; m++) {
                x = i + moves[m].x;
                y = j + moves[m].y;

                if(x < size && x >= 0 && y < size && y >= 0) {
                    degrees[y*size +x]--;
                }
            } // end for each move
        } // end for each row
    } // end for each column
}


/*
 * Fill array with zeros
 * Size is number of elements in the array
 */
void initArray(int* arr, int size) {
    int i;
    for(i = 0; i < size; i++) {
        arr[i] = 0;
    }
}


/*
 * Used for lower bound calculation
 * Adds every other number after the move number to the current row or
 *     column total for every unfilled spot in that row or column
 */
int next(int unfilled, int start) {
    int total = 0, i;
    for(i = 0; i < unfilled; i++) {
        total+= start + (i)*2 + 1;
    }
    return total;
}


/*
 * Used for upper bound calculation
 * Adds every other number counting down from the number of possible moves to
 *    the current row or column for evert unfilled spot in that row or column
 */
int last(int unfilled, int size) {
    int total = 0, i;
    for(i = 0; i < unfilled; i++) {
        total += size*size - i*2;
    }
    return total;
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
 * Generate possible moves sorted to widen bounds
 * The sortedMoves parameter will be filled with the possible moves
 * The number of valid moves in sortedMoves will be returned
 */
int getMovesByBounds(int size, struct point start, struct point* sortedMoves, int spotNum) {
    int x, y, m, i, j;

    // Possible changes to the start point to create moves
    struct point moves[] = {{2, -1}, {2, 1}, {1, -2}, {1, 2}, {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}};
    int numMoves = 0;

    for(m = 0; m < 8; m++) {

        // Calculate possible moves
        moves[m].x += start.x;
        moves[m].y += start.y;

        x = moves[m].x;
        y = moves[m].y;

        if(x < size && x >= 0 && y < size && y >= 0 && array[y*size+x] == 0) { // if spot is valid and open
            numMoves++;

            // Weigh based on sum of vertical and horizontal lower bounds
            moves[m].d = totals[x] + next(size-filled[x], spotNum) + totals[y+size] + next(size-filled[y+size], spotNum);
        }
        else moves[m].d = 0; // Weigh so that invalid moves won't be in the generated moves array
    }

    int max;
    struct point temp;

    // sort moves in degreasing lower bound
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
 * Generates a semi-magical knight's tour recursively
 * Fills the global array variable with the generated tour
 * Returns 1 for success or 0 for failure
 */
int findTour(int size, struct point start, int spotNum, int sortMethod) {
    array[start.y*size+start.x] = spotNum;

    // Return 1 if tour is complete
    if(spotNum == size*size) return 1;

    // Update status of rows and columns for bound calculation
    totals[start.x] += spotNum;
    totals[size+start.y] += spotNum;
    filled[start.x]++;
    filled[size+start.y]++;

    int numMoves, result, x, y, m, i, t;

    // Initialize bounds
    int lower = totals[0] + next(size-filled[0], spotNum);
    int upper = totals[0] + last(size-filled[0], size);

    // Calculate upper and lower bounds
    for(i = 1; i < size*2; i++) {
        t = totals[i] + last(size-filled[i], size);
        if(t < upper) upper = t;
        t = totals[i] + next(size-filled[i], spotNum);
        if(t > lower) lower = t;
    }

    if(lower <= upper) { // if bounds indicate progress on tour is valid
        int xChange[8] = {-2, -1, 1, 2, 2, 1, -1, -2};
        int yChange[8] = {1, 2, 2, 1, -1, -2, -2, -1};
        struct point moves[8];

        // Get possible moves
        if(sortMethod == DEGREE) numMoves = getMovesByDegree(size, start, moves, spotNum);
        else numMoves = getMovesByBounds(size, start, moves, spotNum);

        for(m = 0; m < numMoves; m++) {
            if(array[moves[m].y*size+moves[m].x] != 0) continue;

            // Recursive call to move to next possible spot
            result = findTour(size, moves[m], spotNum+1, sortMethod);
            if(result) return result;
        }

        if(sortMethod == DEGREE) {
            // Update bounds to open current spot
            for(m = 0; m < 8; m++) {
                x = start.x + xChange[m];
                y = start.y + yChange[m];
                if(x < size && x >= 0 && y < size && y >= 0) {
                    degrees[y*size+x]++;
                }
            }
        }
    } // end if bounds indicate progress on tour is valid

    // Backtrack if out of bounds or no more moves
    totals[start.x] -= spotNum;
    totals[size+start.y] -= spotNum;
    filled[start.x]--;
    filled[size+start.y]--;
    array[start.y*size+start.x] = 0;
    return 0;
}


/*
 * Loads a partial tour into the array for a tour to be generated
 * Used to test generating tours in less time
 * Returns the index of the desired starting spot
 */
int fillTour(int max, int tourNum) {
    // Beverley,1848 a8-c1
    int tour1[] = {1, 30, 47, 52, 5, 28, 43, 54,
                48, 51, 2, 29, 44, 53, 6, 27,
                31, 46, 49, 4, 25, 8, 55, 42,
                50, 3, 32, 45, 56, 41, 26, 7,
                33, 62, 15, 20, 9, 24, 39, 58,
                16, 19, 34, 61, 40, 57, 10, 23,
                63, 14, 17, 36, 21, 12, 59, 38,
                18, 35, 64, 13, 60, 37, 22, 11};

     // Mackay/Meyrignac/Stertenbrink, 2003 b3-d2
     int tour2[] = {59, 30, 35, 24, 57, 22, 15, 18,
                  36, 25, 58, 29, 16, 19, 56, 21,
                  31, 60, 27, 34, 23, 54, 17, 14,
                  26, 37, 32, 49, 28, 13, 20, 55,
                  39, 4, 61, 12, 33, 48, 53, 10,
                  62, 1, 38, 7, 50, 11, 44, 47,
                  5, 40, 3, 64, 45, 42, 9, 52,
                  2, 63, 6, 41, 8, 51, 46, 43 };

     // Jelliss, 2003 d5-f4
     int tour3[] = {11, 46, 51, 40, 9, 38, 31, 34,
                 52, 41, 10, 45, 32, 35, 8, 37,
                 47, 12, 43, 50, 39, 6, 33, 30,
                 42, 53, 48, 1, 44, 29, 36, 7,
                 55, 20, 13, 28, 49, 64, 5, 26,
                 14, 17, 54, 23, 2, 27, 60, 63,
                 21, 56, 19, 16, 61, 58, 25, 4,
                 18, 15, 22, 57, 24, 3, 62, 59};

     // Mackay/Meyrignac/Stertenbrink, 2003 a1-c8
     int tour4[] = {18, 39, 64, 9, 58, 41, 24, 7,
                 63, 10, 17, 40, 23, 8, 57, 42,
                 38, 19, 36, 61, 16, 59, 6, 25,
                 11, 62, 13, 20, 33, 22, 43, 56,
                 50, 37, 32, 35, 60, 15, 26, 5,
                 31, 12, 49, 14, 21, 34, 55, 44,
                 48, 51, 2, 29, 46, 53, 4, 27,
                 1, 30, 47, 52, 3, 28, 45, 54};

    int* tour;
    if(tourNum == 1) tour = tour1;
    else if(tourNum == 2) tour = tour2;
    else if(tourNum == 3) tour = tour3;
    else tour = tour4;

    int i, maxIndex;

    for(i = 0; i<64; i++) {
        if(tour[i] < max) {
            // Fill tour in tour array
            array[i] = tour[i];

            // Fill arrays for bound calculations
            totals[i%8] += tour[i];
            filled[i%8]++;
            totals[8+i/8] += tour[i];
            filled[8+i/8]++;
        }
        if(tour[i] == max) maxIndex = i;
    }

    return maxIndex;
}


// Setup and generate semi-magical knight's tour
void start(int startNum, int board, int sortMethod) {
    int size = 8;

    // Initialize arrays for tour and bound calculation
    array = (int*)malloc(sizeof(int)*size*size);
    initArray(array, size*size);

    degrees = (int*)malloc(sizeof(int)*size*size);

    totals = (int*)malloc(sizeof(int)*size*2);
    filled = (int*)malloc(sizeof(int)*size*2);
    initArray(filled, size*2);
    initArray(totals, size*2);

    // Fill tour partially with existing tour
    int index = fillTour(startNum, board);

    struct point start;
    start.x = index%8;
    start.y = index/8;

    initializeDegrees(size);
    updateDegrees(size);

    // Generate the rest of the knight's tour
    time_t sTime, eTime;
    time(&sTime);
    findTour(size, start, startNum, sortMethod);

    time(&eTime);
    printf("%f\t", difftime(eTime, sTime));

    free(totals);
    free(filled);
    free(degrees);
    free(array);
}

int main() {
   int i, t;

   // Print times for generating tours on 4 different boards, starting at various nodes
   // BOUNDS TIME is for choosing the next moves to widen the bounds
   // DEGREE TIME is for choosing the next moves based on minimal degree
   printf("START:\tBOARD:\tDEGREE TIME:\t BOUNDS TIME:\n");
   for(i = 30; i >= 0; i--) {
       for(t = 1; t <= 4; t++) {
           printf("\n%d\t%d\t", i, t);
           start(i, t, BOUNDS);
           start(i, t, DEGREE);
       }
   }
}


