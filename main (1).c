#include <stdio.h>
#include <stdlib.h>

/* FileName: MAGLTestingLaptop
 * Description: MAGL Coursework : Maze Solving Algorithm for Micromouse Project
 * Author: Louis Robinson @UWE 2025
 * Date: 27/04/2025
 * Version 1
 */
#include "main.h"


int main() {

    //representing the downwards direction.
    int sensor_data[4]; //Left (IR), front (IR or Ultrasonic), Right (IR), Behind (no sensor but can be assumed)
    sensor_data[3] = 0; //setting our backwards "sensor" to register a wall as we start in the corner

    int map[6][8];  //map layout
    int visited_map[6][8];  //understanding where we have gone


    generate_map(map, visited_map); //generate the initial map layout

    while(1){   //while 1 used to represent STM32 Layout
        mazeAlgorithmStart(sensor_data, map, visited_map);  //start our maze solving algorithm
    }

}
int mazeAlgorithmStart(int sensor_data[], int (*map)[8],int (*visited_map)[8]){
    //Our whole algorithm

    int next_node_index[2]; //Next node we want to go to
    int gyroAngle = 0, shifted_by, motorCmd; //gyro angle, shifted by (rotating our sensors to suit the global map orientation), MotorCmd (resulting motor drive)

    gyroAngle = sensorData(sensor_data, gyroAngle); //getting the gyroAngle and sensor values
    shifted_by = localRotation(gyroAngle,sensor_data);  //Interpreting sensor values globally and locally
    open_node(sensor_data); //detecting walls
    motorCmd=next_node(next_node_index, sensor_data,map,visited_map); //understanding which node to go to next
    cmdCalculation(shifted_by, motorCmd); //generating the correct motor command to be sent to the control module of micromouse
    printf("\nNext node Index: %d %d\n", next_node_index[0],next_node_index[1]);
    sensor_data[3] = 500;


    return 0;
}

//updating map due to dead ends
int edit_map(int open_node_Index[], int visited_nodes[], int visited_nodes_index, int (*map)[8]){
    // we know our current node is dead end
    int flag = 0; // finding where our open node is in the visited array and how many nodes we have travelled
    int counter = 0; //increasing the values
    printf("\nStarting edit");
    printf("\nvisited node index = %d \n", visited_nodes_index);
    printf("Open node index = [%d][%d]\n", open_node_Index[0], open_node_Index[1]);

    for (int i = 0; i <= visited_nodes_index; ++i) {
        printf("[%d][%d] \n", visited_nodes[i], visited_nodes[i+1]);
        if(visited_nodes[i] == open_node_Index[0] && visited_nodes[i+1] == open_node_Index[1]){ //searching for open node
            flag = visited_nodes_index-i;
            printf("Nodes Travelled since open node: %d", flag/2);
        }

        if(flag != 0){
            map[visited_nodes[i]][visited_nodes[i+1]] -=counter; //subtracting the amount of nodes travelled from each node
            counter-=2; //reducing the amount of
             //taking away two in order to pop the visited nodes from the stack (over writing them)
        }   //allowing us to accurately reset node values to scale

        i++; //increasing i by one manually to jump by 2, (per node)
    }

    //we need to know the last node with multiple options
    return flag;
}

//generating the initial map layout
int generate_map(int (*map)[8],int (*visited_map)[8]){
    int value = 7;
    int half_way_flag = 1, constant =1;

    for (int i = 0; i < 6; ++i) {
        if(i>3){
            half_way_flag = -1; //to reverse the order
        }else if(i==3){
            value =+5;
        }
        for (int j = 0; j < 8; j++) {//for each column in that row

            if(j<4){
                value -= constant*half_way_flag;
            }else if (j>4){
                value += constant*half_way_flag;
            }else{
                value = value;
            }
            half_way_flag =1;//resetting the flag

            //setting up map
            map[i][j] = value;
            //setting up visited nodes map
            visited_map[i][j]= value;

            printf("[%d]", value);
        }

        printf("\n");
    }

    //edit_map(map);
    return 0;
}

//finding the next node to go to
int next_node(int next_node_index[2], int sensor_data[4], int(*map)[8],int (*visited_map)[8]){
    //declaring our statics
    static int current_r = 5, current_c = 0; //current row and column index
    static int visited_nodes[30]; // all the nodes we have visited
    static int visited_nodes_index = 2; //the index of the visited nodes array
    static int open_node_Index[2]; //node to fall back to incase dead end
    //flags
    static int arrived_at_centre_flag = 0, pop_flag=0;
    //declaring our dynamic
    int available_nodes[4];
    int motorcmd = 10; //default value, if it is returned as 10... do nothing

    //setting our first visited node
    visited_nodes[0] = 5; visited_nodes[1] = 0;

   //Popping nodes that lead to a dead end
    if(current_r == open_node_Index[0] && current_c == open_node_Index[1] && pop_flag>0){
        printf("Pop Flag active: Popped nodes= %d", pop_flag/2);
        visited_nodes_index -=(pop_flag*2)-4;
        pop_flag=0; //resetting our popped flag
    }
    //getting current nodes value
    int current_node_value = map[current_r][current_c];

    //movement opportunities
    //AS WE CANT ROTATE IN THE SIMULATION, THIS WILL NEED TO BE ADJUSTED FOR THE STM32

    int counter =0; //understanding if we are at a dead end
    for (int i = 0; i < 4; ++i) {
       printf("Sensor data [%d] = %d", i, sensor_data[i]);
        if(sensor_data[i] >0){
            switch(i){
                case 0:
                    available_nodes[i]=map[current_r][current_c-1];
                    counter++;
                    break;

                case 1:
                    available_nodes[i]=map[current_r-1][current_c];
                    counter++;
                    break;

                case 2:
                    available_nodes[i]=map[current_r][current_c+1];
                    break;

                case 3:
                    available_nodes[i]=map[current_r+1][current_c];
                    counter++;
                    break;
                default:
                    break;

            }
        }else{
            available_nodes[i] =48; //otherwise, there is a wall! //48 = 6*8
        }
    }
    //if one of each axis is reporting open route...
    if((available_nodes[0]<48||available_nodes[1]<48)&&(available_nodes[2]<48||available_nodes[3]<48)){
        //set current node as an open node (has optional paths)
        open_node_Index[0] = current_r;                     //we have an open node!
        open_node_Index[1] = current_c;

    }else if (counter == 0){
        //Sensors on both axis are recording walls (dead end)
        printf("DEAD END------");
        pop_flag = edit_map(open_node_Index, visited_nodes, visited_nodes_index, map); // we therefore want to update the maps values, taking us back to the last available node
    }

//


    printf("\n Visited_nodes\n");
    visited_map[current_r][current_c] = 0;
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 8; ++j) {
            printf("[%d]",visited_map[i][j]);
        }
        printf("\n");

    }

    //finding the highest value
    //create a counter in else, if counter reaches 4, set id to 4.
    //now we know that we cant move...
    //we need to find our two available routes...
    //if it doesnt match the previous visited node[index], set its value -1 and move to it!
    int minimia_fixed_flag=0;
    while(minimia_fixed_flag == 0) {

        //save matching values ID to an array and generate a random number to select it.
        int id;
        counter = 0;
        for (int i = 0; i < 4; ++i) {
            if (current_node_value >= available_nodes[i]) {
                printf("Comparing current: %d and next: %d\n", current_node_value, available_nodes[i]);
                current_node_value = available_nodes[i];
                printf("New current = %d\n", current_node_value);
                id = i;
                printf("ID = %d", id);
            } else if(current_node_value == available_nodes[i]) {

            }else{
                counter++; //counting the amount of refused options to move
                printf("Comparing current: %d and next: %d\n", current_node_value, available_nodes[i]);
                printf("New current = %d\n", current_node_value);
            }
        }
        printf("\n%d\n", id);

        if (counter == 4) {
            id = 4;
        }
        //could be a problem with arriving at the middle of the maze?

        //finding our next node to go to
        switch (id) {

            case 0:
                printf("Move: left");
                next_node_index[0] = current_r;
                next_node_index[1] = current_c - 1;
                //out the loop
                minimia_fixed_flag = 1;
                motorcmd =0;
                break;

            case 1:
                printf("Move: up");
                next_node_index[0] = current_r - 1;
                next_node_index[1] = current_c;
                //out the loop
                minimia_fixed_flag = 1;
                motorcmd =1;
                break;

            case 2:
                printf("Move: right");
                next_node_index[0] = current_r;
                next_node_index[1] = current_c + 1;
                //out the loop
                minimia_fixed_flag = 1;
                motorcmd =2;
                break;

            case 3:
                printf("Move: down");
                next_node_index[0] = current_r + 1;
                next_node_index[1] = current_c;
                //out the loop
                minimia_fixed_flag = 1;
                motorcmd = 3;
                break;

            case 4:
                if(map[current_r][current_c] != -48){    //if we are making our way to the centre
                    //increasing our local minima to prevent us from getting stuck!
                    map[current_r][current_c] += 1;
                    printf("\nMinima boosted: + 1!\n");
                    printf("\ncurrent_r = %d current_c = %d\n", current_r, current_c);
                    current_node_value = map[current_r][current_c];
                }else{
                    //the only minima on the way back will be the start node! Meaning we have completed the journey back
                    printf("I've done it! :D ");
                    exit(1); //STM32 VERSION: HAL_DELAY(2 SECONDS OR WHATEVER TIME IS)
                }


                break;

        }
    }

    current_r= next_node_index[0];
    current_c= next_node_index[1];

    printf("\nnew current node = [%d][%d]\n", next_node_index[0],next_node_index[1]);

    if(arrived_at_centre_flag == 0){ //once we have arrived at the centre we no longer need to make our path as the map has been edited

        //adding the visted nodes to a static list
        printf("Current node index inside the array = %d",visited_nodes_index);
        visited_nodes[visited_nodes_index]=next_node_index[0];
        visited_nodes_index+=1;
        printf(" and %d\n", visited_nodes_index);
        visited_nodes[visited_nodes_index]=next_node_index[1];
        visited_nodes_index+=1;

        //creating our list of visited nodes to help us find the way back!
        printf("\nPrevious node going back down = [%d][%d]",visited_nodes[visited_nodes_index-4],visited_nodes[visited_nodes_index-3]);
        map[visited_nodes[visited_nodes_index-4]][visited_nodes[visited_nodes_index-3]] -=2;
        printf(" Its value: %d\n", map[visited_nodes[visited_nodes_index-4]][visited_nodes[visited_nodes_index-3]]);

        printf("\nPrevious node going up = [%d][%d]",visited_nodes[visited_nodes_index-4],visited_nodes[visited_nodes_index-3]);
        map[visited_nodes[visited_nodes_index-4]][visited_nodes[visited_nodes_index-3]] +=4;
        printf(" Its value: %d", map[visited_nodes[visited_nodes_index-4]][visited_nodes[visited_nodes_index-3]]);
        //prevent_backstep(visited_nodes, visited_nodes_index);

    }

    //if we have arrived at the middle
    if(current_node_value == 1){
        //calling the arrived at centre function
        arrived_at_centre(visited_nodes, visited_nodes_index, map);
        arrived_at_centre_flag =1;

    }

    printf("\n Next_node\n");
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 8; ++j) {
            printf("[%d]",map[i][j]);
        }
        printf("\n");

    }

    return motorcmd;

}
int open_node(int sensor_data[]){
    //converting sensor data into a binary value (wall or no wall)

    for (int i = 0; i < 4; ++i) {
        if(sensor_data[i]<250){
            sensor_data[i]=0;   // 0 = wall is present (cant go there)
        }else{
            sensor_data[i]=1; // 1 = wall is not present (open node)
        }

    }

    return 0;
}


//arriving at the centre
int arrived_at_centre(int visited_nodes[], int visited_nodes_index, int (*map)[8]){
    //creating our priority path on the map to fall back to
    int counter = 48;
    printf("\n All the visited nodes: \n");
    for (int i = 0; i < visited_nodes_index; ++i) {
        printf("[%d][%d], ",visited_nodes[i],visited_nodes[i+1]);
        map[visited_nodes[i]][visited_nodes[i+1]] =(-counter); //subtracting the amount of nodes travelled from each node
        counter-=1; //reducing the amount of
        //allowing us to accurately reset node values to scale
        i++; //increasing i by one manually to jump by 2, (per node)
    }


    return 0;
}

//simulating maze sensor data
int sensorData(int sensor_data[], int gyroAngle){

    printf("IR left, Front, IR Right");
    scanf("%d %d %d", &sensor_data[0],&sensor_data[1],&sensor_data[2]);
    printf("What is the gyro angle");
    scanf("%d", &gyroAngle);


    return gyroAngle;
}

int localRotation(int gyroAngle, int sensor_data[]){
   //Translating our local rotation into global orientation
    static int previous_angle =0;
    int change_in_angle = gyroAngle,shifted_by = 4;
    printf("\n change = %d, gyro = %d, prev = %d\n", change_in_angle,gyroAngle,previous_angle);

    //Doesnt take into account current rotation
    if(87<=change_in_angle && change_in_angle<=92){
        //We have turned right
        shifted_by = 1;
    }else if(177<= change_in_angle&& change_in_angle <= 183) {
        //We have turned around
        shifted_by = 2;
        //we have done a 180 so wall behind us
        sensor_data[3] = 0;
    }else if(267<= change_in_angle&& change_in_angle <= 273){
        //We have turned left
        shifted_by = 3;
    }
    //otherwise, shifted stays in the same format

    // Rotate the array by n position
    printf("\nShifting by %d\n", shifted_by);
    for (int i = 0; i < shifted_by; ++i) {
        int temp = sensor_data[3];
        for (int j = 2; j >= 0; j--) {
            sensor_data[j+1] = sensor_data[j];
        }
        sensor_data[0] = temp;

    }
    printf("1.%d 2.%d 3.%d 4.%d", sensor_data[0],sensor_data[1],sensor_data[2],sensor_data[3]);
    previous_angle = gyroAngle;

    return shifted_by;
}
int cmdCalculation(int shifted_by, int globalCmd){
    int motorCmd;

    motorCmd =  shifted_by-globalCmd;

    switch (motorCmd) {
        case 0:
            printf("Going Left then forward");
            break;
        case 1:
            printf("180 Spin and going forward");
            break;
        case 2:
            printf("Right then forward");
            break;
        case -2:
            printf("Right then forward");
            break;
        case 3:
            printf("Going forward");
            break;
        case -1:
            printf("Going forward");
            break;
    }


    return 0;
}