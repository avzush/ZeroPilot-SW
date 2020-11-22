/**
 * Waypoint Manager Methods and Helpers Implementation
 * Author: Dhruv Rawat
 * Created: November 2020
 * Last Updated: November 2020 (Dhruv)
 */

#include "../Inc/waypointManager.hpp"


/*** INITIALIZATION ***/

WaypointManager::WaypointManager() {
    nextAssignedId = 0;
}

void WaypointManager::initialize_waypoint_manager(_PathData ** initialWaypoints, int numberOfWaypoints, _PathData *currentLocation) {
    homeBase = *currentLocation;

    numWaypoints = numberOfWaypoints;
    nextFilledIndex = 0;

    // Initializes the waypointBuffer array
    for (int i = 0; i < numWaypoints; i++) {
        waypointBuffer[i] = initialWaypoints[i]; // Sets the element in the waypointBuffer
        waypointBufferStatus[i] = FULL;
        nextFilledIndex = i + 1;
    }

    // Links waypoints together
    for (int i = 0; i < numWaypoints; i++) {
        if (i == 0) { // If first waypoint, link to next one only
            waypointBuffer[i]->next = waypointBuffer[i+1];
        } else if (i == numWaypoints - 1) { // If last waypoint, link to previous one only
            waypointBuffer[i]->previous = waypointBuffer[i-1];
        } else {
            waypointBuffer[i]->next = waypointBuffer[i+1];
            waypointBuffer[i]->previous = waypointBuffer[i-1];
        }
    }

    errorStatus = WAYPOINT_SUCCESS;
}

_PathData* WaypointManager::initialize_waypoint() {
    _PathData* waypoint = new _PathData; // Create new waypoint in the heap
    waypoint->waypointId = nextAssignedId++; // Set ID and increment
    // Set next and previous waypoints to empty for now
    waypoint->next = 0;
    waypoint->previous = 0;

    return waypoint;
}

_PathData* WaypointManager::initialize_waypoint(long double longitude, long double latitude, int altitude, int waypointType) {
    _PathData* waypoint = new _PathData; // Create new waypoint in the heap
    waypoint->waypointId = nextAssignedId; // Set ID and increment
    nextAssignedId++;
    waypoint->latitude = latitude;
    waypoint->longitude = longitude;
    waypoint->altitude = altitude;
    waypoint->waypointType = waypointType;
    // Set next and previous waypoints to empty for now
    waypoint->next = 0;
    waypoint->previous = 0;

    return waypoint;
}

_PathData* WaypointManager::initialize_waypoint(long double longitude, long double latitude, int altitude, int waypointType, float turnRadius) {
    _PathData* waypoint = new _PathData; // Create new waypoint in the heap
    waypoint->waypointId = nextAssignedId; // Set ID and increment
    nextAssignedId++;
    waypoint->latitude = latitude;
    waypoint->longitude = longitude;
    waypoint->altitude = altitude;
    waypoint->waypointType = waypointType;
    waypoint->turnRadius = turnRadius;
    // Set next and previous waypoints to empty for now
    waypoint->next = 0;
    waypoint->previous = 0;

    return waypoint;
}


/*** UNIVERSAL HELPERS (universal to this file, ofc) ***/


int WaypointManager::get_waypoint_index_from_id(int waypointId) {
    for (int i = 0; i < PATH_BUFFER_SIZE; i++) {
        if(waypointBufferStatus[i] == FREE) { // If array is empty at the specified index, waypoint is not in buffer
            return -1;
        }
        if(waypointBuffer[i]->waypointId == waypointId) { // If waypoint is found, return index
            return i;
        }
    }

    return -1; // If waypoint is not found
}

void WaypointManager::get_coordinates(long double longitude, long double latitude, float* xyCoordinates) {
    xyCoordinates[0] = get_distance(RELATIVE_LATITUDE, RELATIVE_LONGITUDE, RELATIVE_LATITUDE, longitude); //Calculates longitude (x coordinate) relative to defined origin (RELATIVE_LONGITUDE, RELATIVE_LATITUDE)
    xyCoordinates[1] = get_distance(RELATIVE_LATITUDE, RELATIVE_LONGITUDE, latitude, RELATIVE_LONGITUDE); //Calculates latitude (y coordinate) relative to defined origin (RELATIVE_LONGITUDE, RELATIVE_LATITUDE)
}

float WaypointManager::get_distance(long double lat1, long double lon1, long double lat2, long double lon2) {
    // Longitude and latitude stored in degrees
    // This calculation uses the Haversine formula
    long double change_in_Lat = deg2rad(lat2 - lat1); //Converts change in latitude to radians
    long double change_in_lon = deg2rad(lon2 - lon1); //Converts change in longitude to radians

    double haversine_ans = sin(change_in_Lat / 2) * sin(change_in_Lat / 2) + cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * sin(change_in_lon / 2) * sin(change_in_lon / 2); // In kilometers

    if ((change_in_Lat >= 0 && change_in_lon >=0)||(change_in_Lat < 0 && change_in_lon < 0)){
        return EARTH_RADIUS * (2 * atan2(sqrt(haversine_ans),sqrt(1 - haversine_ans))) * 1000; //Multiply by 1000 to convert to metres
    } else { // If result is negative.
        return EARTH_RADIUS * (2 * atan2(sqrt(haversine_ans),sqrt(1 - haversine_ans))) * -1000;
    }
}


/*** NAVIGATION ***/


_WaypointStatus WaypointManager::get_next_directions(_WaypointManager_Data_In currentStatus, _WaypointManager_Data_Out *Data) {
    if (goingHome) { // If plane was instructed to go back to base (and is awaiting for waypointBuffer to be updated)
        // Do stuff
        dataIsNew = true;
        update_return_data(Data); // Updates the return structure
        return errorCode;
    }

    if (inHold) {   // If plane is currently circling and waiting for commands
        // Do stuff
        dataIsNew = true;
        update_return_data(Data); // Updates the return structure
        return errorCode;
    }

    dataIsNew = true;
    update_return_data(Data); // Updates the return structure
    return errorCode;
}

void WaypointManager::update_return_data(_WaypointManager_Data_Out *Data) {
    Data->desiredHeading = desiredHeading;
    Data->desiredAltitude = desiredAltitude;
    Data->distanceToNextWaypoint = distanceToNextWaypoint;
    Data->radius = turnRadius;
    Data->turnDirection = turnDirection;
    Data->errorCode = errorCode;
    Data->isDataNew = dataIsNew;
    dataIsNew = false;

    // Not setting time of data yet bc I think we need to come up with a way to get it???
}

void WaypointManager::start_circling(float radius, int direction, bool cancelTurning) {
    if (!cancelTurning) {
        inHold = true;
    } else {
        inHold = false;
    }

    turnRadius = radius;
    turnDirection = direction;
}

void WaypointManager::head_home() {
    if (!goingHome) {
        clear_path_nodes(); // Clears path nodes so state machine can input new flight path
        goingHome = true;
    } else {
        goingHome = false;
    }
}

void WaypointManager::follow_waypoints(_PathData * currentWaypoint, float* position, float heading) {

}

void WaypointManager::follow_line_segment(_PathData * currentWaypoint, float* position, float heading) {

}

void WaypointManager::follow_last_line_segment(_PathData * currentWaypoint, float* position, float heading) {

}

void WaypointManager::follow_orbit(float* orbitCenter, float radius, char direction, float* position, float heading) {

}

void WaypointManager::follow_straight_path(float* waypointDirection, float* targetWaypoint, float* position, float heading) {

}

float WaypointManager::maintain_altitude(_PathData* currentPath) {

}


/*** FLIGHT PATH MANAGEMENT ***/


_WaypointStatus WaypointManager::update_path_nodes(_PathData * waypoint, _WaypointBufferUpdateType updateType, int waypointId, int previousId, int nextId) {
    if (numWaypoints == PATH_BUFFER_SIZE && (updateType == APPEND_WAYPONT || updateType == INSERT_WAYPOINT)) { // If array is already full, if we insert it will overflow
        return UNDEFINED_FAILURE;
    }

    // Conducts a different operation based on the update type
    if (updateType == APPEND_WAYPONT) {
        errorCode = append_waypoint(waypoint);
    } else if (updateType == INSERT_WAYPOINT) {
        errorCode = insert_new_waypoint(waypoint, previousId, nextId);
    } else if (updateType == UPDATE_WAYPOINT) {
        errorCode = update_waypoint(waypoint, waypointId);
    } else if (updateType == DELETE_WAYPOINT) {
        errorCode = delete_waypoint(waypointId);
    }

    return errorCode;
}

void WaypointManager::clear_path_nodes() {
    for(int i = 0; i < PATH_BUFFER_SIZE; i++) {
        if (waypointBufferStatus[i] == FULL) { // If array element has a waypoint in it
            int destroyedId = destroy_waypoint(waypointBuffer[i]); // Remove waypoint from the heap
        }
        waypointBufferStatus[i] = FREE; // Set array element free
        waypointBuffer[i] = 0; //Set buffer element to empty struct
    }
    // Resets buffer status variables
    numWaypoints = 0;
    nextFilledIndex = 0;
    nextAssignedId = 0;
}

int WaypointManager::destroy_waypoint(_PathData *waypoint) {
    int destroyedId = waypoint->waypointId;
    delete waypoint; // Free heap memory
    return destroyedId;
}

_WaypointStatus WaypointManager::append_waypoint(_PathData * newWaypoint) {
    int previousIndex = 0;

    previousIndex = nextFilledIndex - 1;

    // Before initializing elements, checks if new waypoint is not a duplicate
    if (previousIndex != -1 && waypointBuffer[previousIndex]->latitude == newWaypoint->latitude && waypointBuffer[previousIndex]->longitude == newWaypoint->longitude) {
        return UNDEFINED_FAILURE;
    }

    waypointBuffer[nextFilledIndex] = newWaypoint;
    waypointBufferStatus[nextFilledIndex] = FULL;

    if (previousIndex == -1) { //If we are initializing the first element
        nextFilledIndex++;
        numWaypoints++;

        return WAYPOINT_SUCCESS;
    }

    // Links previous waypoint with current one
    waypointBuffer[nextFilledIndex]->previous = waypointBuffer[previousIndex];
    waypointBuffer[previousIndex]->next = waypointBuffer[nextFilledIndex];

    nextFilledIndex++;
    numWaypoints++;

    return WAYPOINT_SUCCESS;
}

_WaypointStatus WaypointManager::insert_new_waypoint(_PathData* newWaypoint, int previousId, int nextId) {
    int nextIndex = get_waypoint_index_from_id(nextId);
    int previousIndex = get_waypoint_index_from_id(previousId);

    // If any of the waypoints could not be found. Or, if the two IDs do not correspond to adjacent elements in waypointBuffer[]
    if (nextIndex == -1 || previousIndex == -1 || nextIndex - 1 != previousIndex){
        return UNDEFINED_FAILURE;
    }

    // Adjusts array. Starts at second last element
    for (int i = PATH_BUFFER_SIZE - 2; i >= nextIndex; i--) {
        if (waypointBufferStatus[i] == FULL) { // If current element is initialized
            waypointBuffer[i+1] = waypointBuffer[i]; // Sets next element to current element
            waypointBufferStatus[i+1] = FULL; // Updates state array
        }
    }

    // Put new waypoint in buffer
    waypointBuffer[nextIndex] = newWaypoint;
    waypointBufferStatus[nextIndex] = FULL;

    // Links waypoints together
    waypointBuffer[nextIndex]->next =  waypointBuffer[nextIndex+1];
    waypointBuffer[nextIndex]->previous =  waypointBuffer[previousIndex];
    waypointBuffer[previousIndex]->next = newWaypoint;
    waypointBuffer[nextIndex+1]->previous = newWaypoint;

    return WAYPOINT_SUCCESS;
}

_WaypointStatus WaypointManager::delete_waypoint(int waypointId) {
    int waypointIndex = get_waypoint_index_from_id(waypointId);

    std::cout << waypointIndex << std::endl;

    if (waypointIndex == -1) {
        return UNDEFINED_FAILURE;
    }

    _PathData* waypointToDelete = waypointBuffer[waypointIndex];

    // Links previous and next buffers together
    if (waypointIndex == 0) { //First element
        waypointBuffer[waypointIndex + 1]->previous = 0;
    } else if (waypointIndex == PATH_BUFFER_SIZE - 1 || waypointBufferStatus[waypointIndex+1] == FREE) { // Last element
        waypointBuffer[waypointIndex - 1]->next = 0;
    } else if (waypointBufferStatus[waypointIndex + 1] == FULL){ // Ensures that the next index is
        waypointBuffer[waypointIndex-1]->next = waypointBuffer[waypointIndex+1];
        waypointBuffer[waypointIndex+1]->previous = waypointBuffer[waypointIndex-1];
    }

    destroy_waypoint(waypointToDelete); // Frees heap memory

    // Adjusts indeces so there are no empty elements
    for(int i = waypointIndex; i < PATH_BUFFER_SIZE - 1; i++) {
        if (waypointBufferStatus[i+1] == FULL) {
            waypointBuffer[i] = waypointBuffer[i+1];
            if(i == PATH_BUFFER_SIZE - 2) {
                waypointBufferStatus[i+1] = FREE;
            }
        } else if (waypointBufferStatus[i+1] == FREE) {
            waypointBuffer[i] = 0;
            waypointBufferStatus[i] = FREE;
            return WAYPOINT_SUCCESS;
        } else if (waypointBufferStatus[i] == FREE) {
            return WAYPOINT_SUCCESS;
        }
    }

    // Updates array trackers
    numWaypoints--;
    nextFilledIndex--;

    return WAYPOINT_SUCCESS;
}

_WaypointStatus WaypointManager::update_waypoint(_PathData* updatedWaypoint, int waypointId) {
    int waypointIndex = get_waypoint_index_from_id(waypointId);

    if (waypointIndex == -1) {
        return UNDEFINED_FAILURE;
    }

    _PathData * oldWaypoint = waypointBuffer[waypointIndex];
    waypointBuffer[waypointIndex] = updatedWaypoint; // Updates waypoint

    //Links waypoints together
    if (waypointIndex == 0) { // First element
        waypointBuffer[waypointIndex]->next = waypointBuffer[waypointIndex+1];
        waypointBuffer[waypointIndex + 1]->previous = waypointBuffer[waypointIndex];
    } else if (waypointIndex == PATH_BUFFER_SIZE - 1 || waypointBufferStatus[waypointIndex+1] == FREE) { // Last element
        waypointBuffer[waypointIndex]->previous = waypointBuffer[waypointIndex-1];
        waypointBuffer[waypointIndex - 1]->next = waypointBuffer[waypointIndex];
    } else {
        waypointBuffer[waypointIndex]->next = waypointBuffer[waypointIndex+1];
        waypointBuffer[waypointIndex]->previous = waypointBuffer[waypointIndex-1];
        waypointBuffer[waypointIndex - 1]->next = waypointBuffer[waypointIndex];
        waypointBuffer[waypointIndex + 1]->previous = waypointBuffer[waypointIndex];
    }

    destroy_waypoint(oldWaypoint); // Frees old waypoint from heap memory

    return WAYPOINT_SUCCESS;
}


/*** MISCELLANEOUS ***/


_PathData ** WaypointManager::get_waypoint_buffer() {
    return waypointBuffer;
}

_WaypointBufferStatus WaypointManager::get_status_of_index(int index) {
    return waypointBufferStatus[index];
}

