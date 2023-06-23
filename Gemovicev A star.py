import os
import time
import pickle

MazeSaveFolder = 'Mazes'

StartPosition = [1, 1]
GoalPosition = [10, 10]

VisionSize = 1

MazeDesign = [[0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0],
              [0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0],
              [0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0],
              [0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0],
              [1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0],
              [0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0],
              [1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
              [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0],
              [0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1],
              [0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0],
              [0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0],
              [0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1],
              [0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
              [0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0],
              [0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0],
              [0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0],
              [0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0],
              [0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0]]


def addToOpen(openNodes, neighbor):
    for node in openNodes:
        if (neighbor == node and neighbor.f >= node.f):
            return False
    return True


def saveMaze():
    with open(f'{MazeSaveFolder}/Maze_{runNumber}/MazeDesign_{runNumber}.txt', 'w') as file:
        file.write('Maze design:')
        for row in range(len(MazeDesign)):
            file.write('\n')
            for column in range(len(MazeDesign[row])):
                if row == StartPosition[0] and column == StartPosition[1]:
                    file.write('R ')
                elif row == GoalPosition[0] and column == GoalPosition[1]:
                    file.write('G ')
                else:
                    file.write(str(MazeDesign[row][column]) + ' ')


class Node:
    def __init__(self, position=[0, 0], parent=None):
        self.position = position
        self.parent = parent
        self.g = 0
        self.h = 0
        self.f = 0

    def __eq__(self, other):
        return self.position == other.position

    def __lt__(self, other):
        return self.f < other.f

    def __repr__(self):
        return (f'({self.position[0]}, {self.position[1]})')

    def getDistance(self, other):
        xDistance = self.position[0] - other.position[0]
        yDistance = self.position[1] - other.position[1]

        return (xDistance ** 2 + yDistance ** 2) ** .5

    def getClosestNode(self, nodeList):
        currentNode = nodeList[0]

        for node in nodeList[1:]:
            if self.getDistance(node) < self.getDistance(currentNode):
                currentNode = node

        return currentNode

    def getPositionDifference(self, other):
        xDifference = self.position[0] - other.position[0]
        yDifference = self.position[1] - other.position[1]

        return [xDifference, yDifference]

    def inList(self, list):
        try:
            return list[self.position[0]][self.position[1]] in [0, 'G']
        except:
            return True

    def move(self, delta):
        self.position[0] += delta[0]
        self.position[1] += delta[1]


class Maze:
    def __init__(self, saveFolder, startNode=Node([0, 0]), finalNode=Node([1, 1]), maze=[], visionSize=1, runNumber=0,
                 visionType='full', visionLimited=False):
        self.saveFolder = saveFolder + f'/Maze_{runNumber}'

        try:
            os.mkdir(self.saveFolder)
        except:
            pass

        self.runNumber = runNumber

        self.startNode = startNode
        self.currentNode = startNode
        self.finalNode = finalNode

        self.visionType = visionType
        self.visionLimited = visionLimited
        self.visionSize = visionSize

        self.currentVision = []
        self.path = []

        self.currentMaze = maze

        self.currentMaze[finalNode.position[0]][finalNode.position[1]] = 'G'
        self.currentMaze[startNode.position[0]][startNode.position[1]] = 'R'

        self.distanceTraveled = 0
        self.pathLength = 0
        self.totalTime = 0

    def __repr__(self):
        pathLengthStr = f'Path length: {self.pathLength} nodes'
        distanceTraveledStr = f'Distance traveled: {self.distanceTraveled} node lengths'
        totalTimeStr = f'Total time: {round(self.totalTime, 2)} seconds'

        mazeStr = self.getMazeStr()
        infoStr = f'{pathLengthStr} | {distanceTraveledStr} | {totalTimeStr}\n'
        pathStr = f'Path: {[self.startNode.position] + self.path}'

        return (mazeStr + infoStr + pathStr)

    def savePath(self):
        with open(f'{self.saveFolder}/Path_{self.runNumber}.txt', 'w') as file:
            for position in self.path:
                coordinates = str(position[0] - self.startNode.position[0]) + '\n' + str(
                    position[1] - self.startNode.position[1])
                file.write(coordinates)
                file.write('\n')

    def saveInfo(self):
        startNodeStr = f'Start node: {self.startNode}'
        finalNodeStr = f'Final node: {self.finalNode}'
        visionSizeStr = f'Vision size: {self.visionSize} node in every direction'
        pathLengthStr = f'Path length: {self.pathLength} nodes'
        distanceTraveledStr = f'Distance traveled: {round(self.distanceTraveled * .25, 4)} meters'
        totalTimeStr = f'Total time: {round(self.totalTime, 4)} seconds'
        pathStr = f'Path: {self.startNode.position[0], self.startNode.position[1]}'

        for position in self.path:
            pathStr += f', {position[0], position[1]}'

        with open(f'{self.saveFolder}/Info_{self.runNumber}.txt', 'w') as file:
            file.write(
                f'{startNodeStr}\n{finalNodeStr}\n{visionSizeStr}\n{pathLengthStr}\n{distanceTraveledStr}\n{totalTimeStr}\n{pathStr}')

    def saveOccupiedNodes(self):
        occupiedNodes = self.getOccupiedNodes()

        with open(f'{self.saveFolder}/OccupiedNodes_{self.runNumber}.txt', 'w') as file:
            for node in occupiedNodes:
                file.write(f'({node.position[0]}, {node.position[1]})')
                file.write('\n')

    def setCurrentVision(self):
        self.currentVision = []

        minX = max(self.currentNode.position[0] - self.visionSize, 0)
        maxX = max(self.currentNode.position[0] + self.visionSize, 0) + 1
        minY = max(self.currentNode.position[1] - self.visionSize, 0)
        maxY = max(self.currentNode.position[1] + self.visionSize, 0) + 1

        visibleRows = self.currentMaze[minX: maxX][minY: maxY]

        for row in visibleRows:
            self.currentVision.append(row[minY: maxY])

    def getMazeStr(self):
        mazeString = ''
        for row in self.currentMaze:
            mazeString += str(row) + '\n'

        return mazeString

    def getOccupiedNodes(self):
        occupiedNodes = []

        for x in range(len(self.currentMaze)):
            for y in range(len(self.currentMaze[0])):
                if self.currentMaze[x][y] == 1 or self.currentMaze[x][y] == 2:
                    occupiedNodes.append(Node([x, y]))

        return occupiedNodes

    def getFreeNodes(self):
        freeNodes = []

        for x in range(len(self.currentMaze)):
            for y in range(len(self.currentMaze[0])):
                if self.currentMaze[x][y] == 0 or self.currentMaze[x][y] == 'G':
                    freeNodes.append(Node([x, y]))

        return freeNodes

    def getVisibleNodesInFront(self, nodes):
        currentParent = self.currentNode.parent

        visibleNodesInFront = []

        if self.currentNode.getPositionDifference(currentParent) == [-1, 0]:
            pass
        elif self.currentNode.getPositionDifference(currentParent) == [1, 0]:
            pass
        elif self.currentNode.getPositionDifference(currentParent) == [0, -1]:
            pass
        elif self.currentNode.getPositionDifference(currentParent) == [0, 1]:
            pass
        elif self.currentNode.getPositionDifference(currentParent) == [-1, -1]:
            pass
        elif self.currentNode.getPositionDifference(currentParent) == [-1, 1]:
            pass
        elif self.currentNode.getPositionDifference(currentParent) == [1, -1]:
            pass
        elif self.currentNode.getPositionDifference(currentParent) == [1, 1]:
            pass

        return visibleNodesInFront

    def getVisibleNodesForSensors(self, nodes):
        pass

    def getInvisibleNodes(self, nodes):
        pass

    def getVisibleNodes(self, nodes):
        allVisibleNodes = []

        finalVisibleNodes = []

        for node in nodes:
            if self.inVision(node):
                allVisibleNodes.append(node)

        if self.visionType == 'full':
            currentVisibleNodes = allVisibleNodes
        elif self.visionType == 'front':
            currentlVisibleNodes = self.getVisibleNodesInFront()
        elif self.visionType == 'sensor':
            currentVisibleNodes = self.getVisibleNodesForSensors()

        if self.visionLimited:
            invisibleNodes = self.getInvisibleNodes(allVisibleNodes)

            for node in allVisibleNodes:
                if node not in invisibleNodes:
                    finalVisibleNodes.append(node)
        else:
            finalVisibleNodes = currentVisibleNodes

        return finalVisibleNodes

    def getClosestNode(self, goalNode, nodes):
        if goalNode in nodes:
            return goalNode

        probableClosestNode = nodes[0]

        for node in nodes[1:]:
            if goalNode.getDistance(node) < goalNode.getDistance(probableClosestNode):
                probableClosestNode = node

        return probableClosestNode

    def getCurrentGoal(self):
        if self.visionSize * 2 + 1 >= len(self.currentMaze):
            return self.finalNode

        probableGoalNode = self.currentNode

        for xDelta in range(-self.visionSize, self.visionSize + 1):
            for yDelta in range(-self.visionSize, self.visionSize + 1):
                x = self.currentNode.position[0] + xDelta
                y = self.currentNode.position[1] + yDelta

                probableGoalNode = self.finalNode.getClosestNode([probableGoalNode, Node([x, y])])

        visibleFreeNodes = self.getVisibleNodes(self.getFreeNodes())
        probableGoalNode = self.getClosestNode(probableGoalNode, visibleFreeNodes)

        return probableGoalNode

    def clearMaze(self):
        for rowIndex in range(len(self.currentMaze)):
            self.currentMaze[rowIndex] = [0 if element == 2 else element for element in self.currentMaze[rowIndex]]

    def noPossibleMoves(self):
        try:
            freeW = self.currentMaze[self.currentNode.position[0] - 1][self.currentNode.position[1]] == 0
        except:
            freeW = False

        try:
            freeE = self.currentMaze[self.currentNode.position[0] + 1][self.currentNode.position[1]] == 0
        except:
            freeE = False

        try:
            freeN = self.currentMaze[self.currentNode.position[0]][self.currentNode.position[1] - 1] == 0
        except:
            freeN = False

        try:
            freeS = self.currentMaze[self.currentNode.position[0]][self.currentNode.position[1] + 1] == 0
        except:
            freeS = False

        try:
            freeNW = self.currentMaze[self.currentNode.position[0] - 1][self.currentNode.position[1] - 1] == 0
        except:
            freeNW = False

        try:
            freeNE = self.currentMaze[self.currentNode.position[0] + 1][self.currentNode.position[1] - 1] == 0
        except:
            freeNE = False

        try:
            freeSW = self.currentMaze[self.currentNode.position[0] - 1][self.currentNode.position[1] + 1] == 0
        except:
            freeSW = False

        try:
            freeSE = self.currentMaze[self.currentNode.position[0] + 1][self.currentNode.position[1] + 1] == 0
        except:
            freeSE = False

        if not (freeW or freeE or freeN or freeS or freeNW or freeNE or freeSW or freeSE):
            return True

        return False

    def inVision(self, node):
        x = node.position[0]
        y = node.position[1]

        currentX = self.currentNode.position[0]
        currentY = self.currentNode.position[1]

        visionSize = self.visionSize

        if x >= currentX - visionSize and x <= currentX + visionSize and y >= currentY - visionSize and y <= currentY + visionSize:
            return True

        return False

    def aStar(self):
        self.setCurrentVision()

        maze = self.currentVision

        openNodes = []
        closedNodes = []

        startNode = self.currentNode
        goalNode = self.getCurrentGoal()

        openNodes.append(startNode)

        currentNode = startNode

        while currentNode != goalNode:
            openNodes.sort()

            currentNode = openNodes.pop(0)

            closedNodes.append(currentNode)

            x, y = currentNode.position

            neighbors = [[x - 1, y - 1], [x - 1, y], [x - 1, y + 1], [x, y - 1], [x, y + 1], [x + 1, y - 1], [x + 1, y],
                         [x + 1, y + 1]]

            for position in neighbors:
                if position[0] < 0 or position[0] >= len(self.currentMaze) or position[1] < 0 or position[1] >= len(
                        self.currentMaze[0]):
                    continue

                mapValue = self.currentMaze[position[0]][position[1]]

                if mapValue == 1 or mapValue == 2:
                    continue

                neighbor = Node(position, currentNode)

                if neighbor in closedNodes:
                    continue

                neighbor.g = currentNode.g + abs(position[0] - currentNode.position[0]) + abs(
                    position[1] - currentNode.position[1])
                neighbor.h = (neighbor.position[0] - goalNode.position[0]) ** 2 + (
                            neighbor.position[1] - goalNode.position[1]) ** 2

                neighbor.f = neighbor.g + neighbor.h

                if addToOpen(openNodes, neighbor):
                    openNodes.append(neighbor)

        path = []

        while currentNode != startNode:
            path.append(currentNode.position)
            currentNode = currentNode.parent
        return path[::-1]

    def moveCurrentNode(self, newPosition):
        if abs((newPosition[0] - self.currentNode.position[0]) + (newPosition[1] - self.currentNode.position[1])) == 1:
            self.distanceTraveled += 1
        else:
            self.distanceTraveled += 1.41

        self.currentMaze[self.currentNode.position[0]][self.currentNode.position[1]] = 2
        self.currentNode = Node(newPosition)
        self.currentMaze[self.currentNode.position[0]][self.currentNode.position[1]] = 'R'
        self.pathLength += 1

        print(self.getMazeStr())

    def findPath(self):
        self.saveOccupiedNodes()

        start = time.time()

        while self.currentNode != self.finalNode:
            path = self.aStar()
            self.path += path

            for nextPosition in path:
                self.moveCurrentNode(nextPosition)

            if self.noPossibleMoves():
                self.clearMaze()

        end = time.time()

        self.totalTime = end - start

        self.savePath()
        self.saveInfo()


if __name__ == '__main__':
    runNumber = pickle.load(open('RunNumber.pkl', 'rb'))

    Start = Node(StartPosition, None)
    Goal = Node(GoalPosition, None)

    maze = Maze(saveFolder=MazeSaveFolder,
                startNode=Start,
                finalNode=Goal,
                maze=MazeDesign,
                visionSize=VisionSize,
                runNumber=runNumber)

    saveMaze()

    maze.findPath()

    runNumber += 1
    pickle.dump(runNumber, open('RunNumber.pkl', 'wb'))