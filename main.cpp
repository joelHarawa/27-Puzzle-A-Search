#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
using namespace std;

// Global values
int inf = 9999999;
int STATETILES = 27;
int LAYERTILES = 9;
int STATELAYERS = 3;
int LAYERROWS = 3;

// Tile structure
struct Tile {
    int number;
    int row;
    int column;
    int layer;

    //Tile constructor
    Tile(int number, int row, int column, int layer) :
            number(number), row(row), column(column), layer(layer) {}

    Tile(const Tile& other) {
        number = other.number;
        row = other.row;
        column = other.column;
        layer = other.layer;
    }
};

// Structure of each puzzle state
struct State {
    //Collection of tiles in the state
    vector<Tile*> tiles;

    //State constructor
    State(vector<Tile*>& tiles) : tiles(tiles) {}

    //State copy constructor
    State(const State& other) {
        for (int i = 0; i < other.tiles.size(); i++) {
            Tile* tile = new Tile(*other.tiles[i]);
            tiles.push_back(tile);
        }
    }

    //State assignment operator
    State& operator= (const State& rhs) {
        if (this != &rhs) {
            for (int i = 0; i < tiles.size(); i++) {
                delete tiles[i];
            }

            for (int j = 0; j < rhs.tiles.size(); j++) {
                tiles.push_back(new Tile(rhs.tiles[j]->number, rhs.tiles[j]->row, rhs.tiles[j]->column, rhs.tiles[j]->layer));
            }
        }
        return *this;
    }

    //State destructor
    ~State() {
        tiles.clear();
    }

    //State equality operator
    bool operator== (const State& rhs) {
        bool equal = true;
        for (Tile* lhs : tiles) {
            for (Tile* rhs : rhs.tiles) {
                if (lhs->number == rhs->number) {
                    if (lhs->column != rhs->column || lhs->layer != rhs->layer || lhs->row != rhs->row) {return false;}
                }
            }
        }
        return equal;
    }
};

// Structure of each node, each node represents a state
struct Node {
    State* state;
    int heuristic = 0; // h(n)
    int pathCost = 0; // g(n)
    int costToGoal = inf; // f(n)
    char action = 'I';
    Node* parent = nullptr;
    vector<Node*> neighbors;
    Node(State* state) : state(state) {}

    bool operator==(const Node& other) const {
        return *state == *other.state;
    };

};

// Loads input file and creates tiles according to the position of the thing
pair<vector<Tile*>, vector<Tile*>> readPuzzle(ifstream& testFile) {
    vector<Tile*> tiles;
    vector<Tile*> goal;
    string num;

    int row = 1;
    int layer = 1;
    int column = -1;
    int totalTiles = 1;

    // Reads every number in the input text file and organizes puzzle state
    while (testFile >> num) {
        Tile* tile = new Tile(stoi(num), row, column, layer);
        if (totalTiles <= STATETILES) {
            tiles.push_back(tile);
        } else {
            goal.push_back(tile);
        }
        if (column != 1) {
            column++;
        } else {
            column = -1;
            if (row != -1) {
                row--;
            } else {
                row = 1;
            }
        }
        if (totalTiles % LAYERTILES == 0) {
            layer--;
        }
        if (totalTiles == STATETILES) {
            layer = 1;
        }
        totalTiles++;
    }

    pair<vector<Tile*>, vector<Tile*>> result;
    result.first = tiles;
    result.second = goal;
    return result;
};

//Looks for matching tile in tiles list to swap position with
pair<Tile*, Tile*> findSwapTile(vector<Tile*> tiles, string& action) {
    Tile *blank;
    Tile *swap;
    int j = 0;

    while (j < tiles.size()) {
        if (tiles[j]->number == 0) {
            blank = tiles[j];
            break;
        }
        j += 1;
    }

    for (int i = 0; i < tiles.size(); i++) {
        bool inLineVertically = blank->column == tiles[i]->column && blank->row == tiles[i]->row;
        bool inLineHorizontally = blank->row == tiles[i]->row && blank->layer == tiles[i]->layer;
        bool inLineVertically2 = blank->column == tiles[i]->column && blank->layer == tiles[i]->layer;

        if (action == "up") {
            bool up = blank->layer + 1 == tiles[i]->layer;
            if (inLineVertically && up) {
                swap = tiles[i];
                break;
            }
        } else if (action == "down") {
            bool down = blank->layer - 1 == tiles[i]->layer;
            if (inLineVertically && down) {
                swap = tiles[i];
                break;
            }
        } else if (action == "west") {
            bool west = blank->column - 1 == tiles[i]->column;
            if (inLineHorizontally && west) {
                swap = tiles[i];
                break;
            }
        } else if (action == "east") {
            bool east = blank->column + 1 == tiles[i]->column;
            if (inLineHorizontally && east) {
                swap = tiles[i];
                break;
            }
        } else if (action == "north") {
            bool north = blank->row + 1 == tiles[i]->row;
            if (inLineVertically2 && north) {
                swap = tiles[i];
                break;
            }
        } else if (action == "south") {
            bool south = blank->row - 1 == tiles[i]->row;
            if (inLineVertically2 && south) {
                swap = tiles[i];
                break;
            }
        }
    }

    pair<Tile *, Tile *> swapTiles;
    swapTiles.first = blank;
    swapTiles.second = swap;
    return swapTiles;
}

// Implementation of up action, swaps blank tile with tile in higher layer
State* tileUp(State* state) {
    State* stateCopy = new State(*state);
    string action = "up";
    pair<Tile*, Tile*> swapTiles = findSwapTile(stateCopy->tiles, action);
    swapTiles.first->layer++;
    swapTiles.second->layer--;
    return stateCopy;
}

// Implementation of down action, swaps blank tile with tile in lower layer
State* tileDown(State* state) {
    State* stateCopy = new State(*state);
    string action = "down";
    pair<Tile*, Tile*> swapTiles = findSwapTile(stateCopy->tiles, action);
    swapTiles.first->layer--;
    swapTiles.second->layer++;
    return stateCopy;
}

// Implementation of west action, swaps blank tile with tile in west column
State* tileWest(State* state) {
    State* stateCopy = new State(*state);
    string action = "west";
    pair<Tile*, Tile*> swapTiles = findSwapTile(stateCopy->tiles, action);
    swapTiles.first->column--;
    swapTiles.second->column++;
    return stateCopy;
}

// Implementation of east action, swaps blank tile with tile in east column
State* tileEast(State* state) {
    State* stateCopy = new State(*state);
    string action = "east";
    pair<Tile*, Tile*> swapTiles = findSwapTile(stateCopy->tiles, action);
    swapTiles.first->column++;
    swapTiles.second->column--;
    return stateCopy;
}

// Implementation of north action, swaps blank tile with tile in north row
State* tileNorth(State* state) {
    State* stateCopy = new State(*state);
    string action = "north";
    pair<Tile*, Tile*> swapTiles = findSwapTile(stateCopy->tiles, action);
    swapTiles.first->row++;
    swapTiles.second->row--;
    return stateCopy;
}

// Implementation of south action, swaps blank tile with tile in south row
State* tileSouth(State* state) {
    State* stateCopy = new State(*state);
    string action = "south";
    pair<Tile*, Tile*> swapTiles = findSwapTile(stateCopy->tiles, action);
    swapTiles.first->row--;
    swapTiles.second->row++;
    return stateCopy;
}

// Finds the sum of the Manhattan distances for every tile
int calculateManhattan(State* currentState, State* goalState) {
    int result = 0;
    for (Tile* currentTile : currentState->tiles) {
        int deltaX, deltaY, deltaZ;
        Tile* matchTile;
        for (Tile* goalTile : goalState->tiles) {
            if (goalTile->number == currentTile->number) {
                matchTile = goalTile;
                break;
            }
        }

        deltaX = abs(currentTile->column - matchTile->column);
        deltaY = abs(currentTile->row - matchTile->row);
        deltaZ = abs(currentTile->layer - matchTile->layer);

        int manhattanDistance = deltaX + deltaY + deltaZ;
        result += manhattanDistance;
    }
    return result;
};

// Boolean check for whether or not a node exists in a list
bool inList(Node* currentNode, vector<Node*>& list) {
    for (Node* node : list) {
        if (*currentNode == *node) {
            return true;
        }
    }
    return false;
}

// Returns node in list
Node* findInList(Node* currentNode, vector<Node*>& list) {
    for (Node* node : list) {
        if (*currentNode == *node) {
            return node;
        }
    }
    return nullptr;
}

// Returns list of neighboring nodes
vector<Node*> expand(Node* currentNode) {
    // Search for the blank tile
    vector<Node*> neighbors;
    Tile* blank;
    for(Tile* currentTile : currentNode->state->tiles) {
        if (currentTile->number == 0) {
            blank = currentTile;
        }
    }

    //Check for valid actions, add Nodes to neighbor
    if (blank->row == 1) {
        Node* neighbor = new Node(tileSouth(currentNode->state));
        neighbor->action = 'S';
        neighbors.push_back(neighbor);
    } else if (blank->row == 0) {
        Node* neighbor1 = new Node(tileSouth(currentNode->state));
        Node* neighbor2 = new Node(tileNorth(currentNode->state));
        neighbor1->action = 'S';
        neighbor2->action = 'N';
        neighbors.push_back(neighbor1);
        neighbors.push_back(neighbor2);
    } else if (blank->row == -1) {
        Node* neighbor = new Node(tileNorth(currentNode->state));
        neighbor->action = 'N';
        neighbors.push_back(neighbor);
    }

    if (blank->layer == 1) {
        Node* neighbor = new Node(tileDown(currentNode->state));
        neighbor->action = 'D';
        neighbors.push_back(neighbor);
    } else if (blank->layer == 0) {
        Node* neighbor1 = new Node(tileUp(currentNode->state));
        Node* neighbor2 = new Node(tileDown(currentNode->state));
        neighbor1->action = 'U';
        neighbor2->action = 'D';
        neighbors.push_back(neighbor1);
        neighbors.push_back(neighbor2);
    } else if (blank->layer == -1) {
        Node* neighbor = new Node(tileUp(currentNode->state));
        neighbor->action = 'U';
        neighbors.push_back(neighbor);
    }

    if (blank->column == 1) {
        Node* neighbor = new Node(tileWest(currentNode->state));
        neighbor->action = 'W';
        neighbors.push_back(neighbor);
    } else if (blank->column == 0) {
        Node* neighbor1 = new Node(tileEast(currentNode->state));
        Node* neighbor2 = new Node(tileWest(currentNode->state));
        neighbor1->action = 'E';
        neighbor2->action = 'W';
        neighbors.push_back(neighbor1);
        neighbors.push_back(neighbor2);
    } else if (blank->column == -1) {
        Node* neighbor = new Node(tileEast(currentNode->state));
        neighbor->action = 'E';
        neighbors.push_back(neighbor);
    }
    return neighbors;
}

// Return vector of actions taken on path to the current node
vector<char>& fillActions(Node* currentNode) {
    vector<char> actions;
    while (currentNode->action != 'I') {
        actions.push_back(currentNode->action);
        currentNode = currentNode->parent;
    }
    return actions;
}

// Display the depth, total nodes, actions, and f costs for the correct path
void writeOutput(Node* currentNode, int nodes, ofstream& output) {
    int depth = 0;
    vector<char> actions;
    vector<int> costs;
    while (currentNode) {
        if (currentNode->parent) {
            depth += 1;
            actions.push_back(currentNode->action);
        }
        costs.push_back(currentNode->costToGoal);
        currentNode = currentNode->parent;
    }
    output << depth << endl;
    output << nodes << endl;
    for (int i = actions.size()-1; i >= 0; i--) {
        output << actions[i] << " ";
    }
    output << endl;
    for (int i = costs.size()-1; i >= 0; i--) {
        output << costs[i] << " ";
    }
}

// Display each initial and goal state
void writeTiles(State* state, ofstream& output) {
    vector<vector<vector<int>>> layers;

    for (int i = 0; i < STATELAYERS; i++) {
        vector<vector<int>> layer;
        layers.push_back(layer);
        for (int j = 0; j < LAYERROWS; j++) {
            vector<int> row = {0, 0, 0};
            layers[i].push_back(row);
        }
    }

    for (int i = 0; i < state->tiles.size(); i++) {
        int layerIndex = state->tiles[i]->layer + 1;
        int rowIndex = state->tiles[i]->row + 1;
        int columnIndex = state->tiles[i]->column + 1;
        layers[layerIndex][rowIndex][columnIndex] = state->tiles[i]->number;
    }

    for (int i = STATELAYERS-1; i >= 0; i--) {
        for (int j = LAYERROWS-1; j >=0;  j--) {
            output << layers[i][j][0] << " " << layers[i][j][1] << " " << layers[i][j][2] << endl;
        }
        output << endl;
    }
}

void search(Node* initial, Node* goal, ofstream& output) {
    // Initialize current node
    int nodes = 1;
    initial->heuristic = calculateManhattan(initial->state, goal->state);
    initial->costToGoal = initial->heuristic;
    vector<Node*> openList = {initial};
    vector<Node*> closedList = {};

    while (!openList.empty()) {
        // Search for smallest Node in openList
        Node* currentNode = openList[0];
        for (Node* node: openList) {
            if (node->costToGoal < currentNode->costToGoal) {
                currentNode = node;
            }
        }

        // Check if goal has been found
        if (*currentNode == *goal) {
            writeOutput(currentNode, nodes, output);
            break;
        }

        auto curr = find(openList.begin(), openList.end(), currentNode);
        openList.erase(curr);
        closedList.push_back(currentNode);

        // Expand the current node
        currentNode->neighbors = expand(currentNode);

        // Loop through neighbors of current node adding to openList and closed list as appropriate
        for (Node* neighbor: currentNode->neighbors) {
            int cost = currentNode->pathCost + 1;
            if (inList(neighbor, closedList)) {
                continue;
            }
            if (inList(neighbor, openList)) {
                Node* oldPath = findInList(neighbor, openList);
                if (cost < oldPath->pathCost) {
                    auto remove = find(openList.begin(), openList.end(), oldPath);
                    openList.erase(remove);
                }
            }
            if  (!inList(neighbor, closedList) && !inList(neighbor, openList)) {
                neighbor->pathCost = cost;
                neighbor->heuristic = calculateManhattan(neighbor->state, goal->state);
                neighbor->costToGoal = neighbor->pathCost + neighbor->heuristic;
                neighbor->parent = currentNode;
                openList.push_back(neighbor);
            }
        }
        closedList.push_back(currentNode);
        nodes += currentNode->neighbors.size();
    }
}

int main() {
    ifstream inputFile;
    ofstream outputFile("output1.txt");
    //Change input here
    inputFile.open("input1.txt");

    //Initialize Initial state, Goal State and respective nodes
    pair<vector<Tile *>, vector<Tile *>> tileVectors = readPuzzle(inputFile);
    State initialState(tileVectors.first);
    State goalState(tileVectors.second);
    Node* initial = new Node(&initialState);
    Node* goal = new Node(&goalState);
    writeTiles(&initialState, outputFile);
    writeTiles(&goalState, outputFile);
    search(initial, goal, outputFile);
    inputFile.close();
    outputFile.close();
    return 0;
}
