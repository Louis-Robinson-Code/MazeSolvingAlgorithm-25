//
// Created by louis on 08/04/2025.
//

#ifndef MAGLTESTINGLAPTOP_MAIN_H
#define MAGLTESTINGLAPTOP_MAIN_H
int sensorData(int sensor_data[],int gyroAngle); //manually writing the sensor data
int open_node(int sensor_data[]);   //detecting where walls are present
int generate_map(int(*map)[8],int (*visited_map)[8]);   //generating the map layout
int next_node(int next_node[], int sensor_data[], int (*map)[8],int (*visited_map)[8]); //locating the next node to go to
int edit_map(int open_node_Index[], int visited_nodes[], int visited_nodes_index, int (*map)[8]); //editing the map based on visited nodes and dead ends
int arrived_at_centre(int visited_nodes[], int visited_nodes_index, int (*map)[8]); //actions to take when we have arrived at the centre (first goal)
int localRotation(int gyroAngle, int sensor_data[]); //converting maze global movements into local rotational movements
int cmdCalculation(int shifted_by, int globalCmd); //generating drive commands based on desired movements
int mazeAlgorithmStart(int sensor_data[], int (*map)[8],int (*visited_map)[8]); //starting the algorithm

#endif //MAGLTESTINGLAPTOP_MAIN_H
