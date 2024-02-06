#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cmath>
#include "functions.h"

using namespace std;

Pixel** createImage(int width, int height) {
  cout << "Start createImage... " << endl;
  
  // Create a one dimensional array on the heap of pointers to Pixels 
  //    that has width elements (i.e. the number of columns)
  Pixel** image = new Pixel*[width];
  
  bool fail = false;
  
  for (int i=0; i < width; ++i) { // loop through each column
    // assign that column to a one dimensional array on the heap of Pixels
    //  that has height elements (i.e. the number of rows)
    image[i] = new Pixel[height];
    
    if (image[i] == nullptr) { // failed to allocate
      fail = true;
    }
  }
  
  if (fail) { // if any allocation fails, clean up and avoid memory leak
    // deallocate any arrays created in for loop
    for (int i=0; i < width; ++i) {
      delete [] image[i]; // deleting nullptr is not a problem
    }
    delete [] image; // delete array of pointers
    return nullptr;
  }
  
  // initialize cells
  //cout << "Initializing cells..." << endl;
  for (int row=0; row<height; ++row) {
    for (int col=0; col<width; ++col) {
      //cout << "(" << col << ", " << row << ")" << endl;
      image[col][row] = { 0, 0, 0 };
    }
  }
  cout << "End createImage... " << endl;
  return image;
}

void deleteImage(Pixel** image, int width) {
  cout << "Start deleteImage..." << endl;
  // avoid memory leak by deleting the array
  for (int i=0; i<width; ++i) {
    delete [] image[i]; // delete each individual array placed on the heap
  }
  delete [] image;
  image = nullptr;
}

// implement for part 1

int* createSeam(int length) {
  int* seam = new int[length];
  for (int i = 0; i < length; i++) {
    seam[i] = 0;
  }
  return seam;
}

void deleteSeam(int* seam) {
  delete[] seam;
  seam = nullptr;
}

bool loadImage(string filename, Pixel** image, int width, int height) {

  ifstream filler;

  // checks if file opens successfully  
  filler.open(filename);
  if (!filler.is_open()){
      cout << "Error: failed to open input file - " << filename << endl;
      return false;
  }

  // checks if file type is P3
  char type[2];
  filler >> type;
  if ((toupper(type[0]) != 'P') || (type[1] != '3')) {
      cout << "Error: type is " << type << " instead of P3" << endl;
      return false;
  }

  // checks that input width and height match the actual width and height of the image
  int actualWidth = 0;
  int actualHeight = 0;
  filler >> actualWidth;
  filler >> actualHeight;
  if (filler.fail()) {
    cout << "Error: read non-integer value" << endl;
    return false;
  }
  if (actualWidth != width){
    cout << "Error: input width (" << width << ") does not match value in file (" << actualWidth << ")" << endl;
    return false;
  }
  if (actualHeight != height){
    cout << "Error: input height (" << height << ") does not match value in file (" << actualHeight << ")" << endl;
    return false;
  }

  // some dumbass requirement i think, where the first number is read as a value but is not meant to be
  int fillerNum;
  filler >> fillerNum;

  // get RGB colour values
  int red = 0;
  int green = 0;
  int blue = 0;

  for (int row = 0; row < height; row++){
    for (int col = 0; col < width; col++){
      
      // check if there are enough colour values to scale the entire image
      if (filler.fail()){
        cout << "Error: not enough color values" << endl;
        return false;
      }

      // clean through the whitespace, then read in the colour values and check whether there are enough of them
      // check if the value is at the end of file, if it is then the program should fail, but you need to add this separately because vscode is fucking stupid
      filler >> ws;
      filler >> red;
      if (filler.fail()){
        cout << "Error: not enough color values" << endl;
        return false;
      }

      filler >> ws;
      filler >> green; 
      if (filler.fail()){
        cout << "Error: not enough color values" << endl;
        return false;
      }

      filler >> ws;
      filler >> blue;
        if (filler.fail()){
          cout << "Error: not enough color values" << endl;
          return false;
      }

      // checks valid colour range
      if (red < 0 || red > 255){
        cout << "Error: invalid color value " << red << endl;
        return false;
      }
      if (green < 0 || green > 255){
        cout << "Error: invalid color value " << green << endl;
        return false;
      }
      if (blue < 0 || blue > 255){
        cout << "Error: invalid color value " << blue << endl;
        return false;
      }

      // checks if the value is of type int
      if (filler.fail()){
        cout << "Error: read non-integer value" << endl;
        return false;
      }

      // if all requirements have been passed, assign the pixel its colour
      image[col][row].r = red;  
      image[col][row].g = green;
      image[col][row].b = blue;
      
    }
  }

  // checks if there are too many colour values, there should be none left
  // if there is another value collected, then there are too many values
  filler >> red;
  if (!filler.eof()){
    cout << "Error: too many color values" << endl;
    return false;
  }

  // if there are NO errors, return the bool function as true
  return true;

}

bool outputImage(string filename, const Pixel*const* image, int width, int height) {
  // open a file and check if it opens successfully
  ofstream filler(filename);
  if (!filler.is_open()){
      cout << "Error: failed to open output file - " << filename << endl;
      return false;
  }

  // include the preamble (file type, width, height, max colour value)
  filler << "P3" << endl;
  filler << width << " " << height << endl;
  filler << 255 << endl;

  // write the colour values of each pixel into the file
  for (int row = 0; row < height; row++){
    for (int col = 0; col < width; col++){
      filler << image[col][row].r << " " << image[col][row].g << " " << image[col][row].b << " ";
    }
    filler << endl;
  }
  
  return true;
}

int energy(const Pixel*const* image, int x, int y, int width, int height) { 
  // return the dual gradient energy of a pixel
  int xGradient = 0, yGradient = 0;
  int xRed = 0, xGreen = 0, xBlue = 0;
  int yRed = 0, yGreen = 0, yBlue = 0;

  // check edge cases, so 2x2 or 1x1
  if ((width == 2 && height == 2) || (width == 1 && height == 1)){
    return 0;
  }
  // check edge cases for 1x2 or 2x1, etc...
  else if (width == 1){
    yRed = abs(image[1][y-1].r - image[1][y+1].r);
    yGreen = abs(image[1][y-1].g - image[1][y+1].g);
    yBlue = abs(image[1][y-1].b - image[1][y+1].b);
  }
  else if (height == 1){
    xRed = abs(image[x-1][1].r - image[x+1][1].r);
    xGreen = abs(image[x-1][1].g - image[x+1][1].g);
    xBlue = abs(image[x-1][1].b - image[x+1][1].b);
  }

  // calculations in the x axis
  // if the pixel is on the left, wrap around to rightmost pixel
  if (x == 0){
    xRed = abs(image[width-1][y].r - image[1][y].r);
    xGreen = abs(image[width-1][y].g - image[1][y].g);
    xBlue = abs(image[width-1][y].b - image[1][y].b);
  }
  // if the pixel is on the right, wrap around to leftmost pixel
  else if (x == width-1){
    xRed = abs(image[width-2][y].r - image[0][y].r);
    xGreen = abs(image[width-2][y].g - image[0][y].g);
    xBlue = abs(image[width-2][y].b - image[0][y].b);
  }
  // if the pixel is not an edgecase, calculate using the adjacent left and right pixel
  else{
    xRed = abs(image[x-1][y].r - image[x+1][y].r);
    xGreen = abs(image[x-1][y].g - image[x+1][y].g);
    xBlue = abs(image[x-1][y].b - image[x+1][y].b);
  }

  // calcuations in the y axis
  // if the pixel is on the the top, wrap around to the bottommost pixel
  if (y == 0){
    yRed = abs(image[x][height-1].r - image[x][1].r);
    yGreen = abs(image[x][height-1].g - image[x][1].g);
    yBlue = abs(image[x][height-1].b - image[x][1].b);
  }
  // if the pixel is on the bottom, wrap aorund to the topmost pixel
  else if (y == height-1){
    yRed = abs(image[x][height-2].r - image[x][0].r);
    yGreen = abs(image[x][height-2].g - image[x][0].g);
    yBlue = abs(image[x][height-2].b - image[x][0].b);
  }
  // if the pixel is not an edgecase, calculate using the adjacent pixels above and below
  else{
    yRed = abs(image[x][y-1].r - image[x][y+1].r);
    yGreen = abs(image[x][y-1].g - image[x][y+1].g);
    yBlue = abs(image[x][y-1].b - image[x][y+1].b);
  }

  // calculate the total gradient energy by summimng the squares
  xGradient = pow(xRed, 2) + pow(xGreen, 2) + pow(xBlue, 2);
  yGradient = pow(yRed, 2) + pow(yGreen, 2) + pow(yBlue, 2);

  return xGradient + yGradient;

} 

// implement for part 2

int loadVerticalSeam(const Pixel*const* image, int start_col, int width, int height, int* seam) {
  // load the vertical seam into the seam array 
  int totalEnergy = 0;
  int nextEnergy = 0;
  int index = 0;
  // energy of the pixels to be compared
  int forwardEnergy = 0;
  int leftEnergy = 0;
  int rightEnergy = 0;

  // set the first seam at the first row and calculate the energy of the first pixel
  seam[0] = start_col;
  totalEnergy = energy(image, start_col, 0, width, height);

  // loop through the rest of the pixels
  for (int row = 1; row < height; row++){

    // pixel is the leftmost pixel
    if (start_col == 0){
      // define the energy of the forward and "left" pixels
      forwardEnergy = energy(image, start_col, row, width, height);
      leftEnergy = energy(image, start_col + 1, row, width, height);
      nextEnergy = min(forwardEnergy, leftEnergy);

      if (nextEnergy == forwardEnergy){
          index = start_col;
      }
      else{
        index = start_col + 1;
      }
    }
  
    // if the pixel is the rightmost pixel
    else if (start_col == width - 1){
      // define the energy of the forward and "right" pixels
      forwardEnergy = energy(image, start_col, row, width, height);
      rightEnergy = energy(image, start_col - 1, row, width, height);
      nextEnergy = min(forwardEnergy, rightEnergy);
      
      if (nextEnergy == forwardEnergy){
        index = start_col;
      }
      else{
        index = start_col - 1;
      }
    }
    
    // if the pixel is not an edgecase
    else{
      // define the energy of the forward, "left", and "right" pixels
      forwardEnergy = energy(image, start_col, row, width, height);
      leftEnergy = energy(image, start_col + 1, row, width, height);
      rightEnergy = energy(image, start_col - 1, row, width, height);
      nextEnergy = min(forwardEnergy, min(rightEnergy, leftEnergy));
      
      if (nextEnergy == forwardEnergy){
        index = start_col;
      }
      else if (nextEnergy == rightEnergy){
        index = start_col - 1;
      }
      else{
        index = start_col + 1;
      }
    }

    // update the total energy and the starting column
    totalEnergy += nextEnergy;
    start_col = index;
    seam[row] = start_col;
  }
  return totalEnergy;
}

int loadHorizontalSeam(const Pixel*const* image, int start_row, int width, int height, int* seam) {
  // load the horizontal seam into the seam array
}

int* findMinVerticalSeam(const Pixel*const* image, int width, int height) {
  // find the minimum vertical seam which is the seam with the lowest energy
  // return a pointer to a seam that is the seam with minimal energy
  int* seam = new int[height];
  int minEnergy = loadVerticalSeam(image, 0, width, height, seam);
  int index = 0;
  // energy of the pixels to be compared
  int forwardEnergy = 0;
  int leftEnergy = 0;
  int rightEnergy = 0;

  // loop through the first row
  for (int col = 0; col < width; col++){
    // calculate the energy of the first pixel
    int totalEnergy = energy(image, col, 0, width, height);

    // loop through the rest of the pixels
    for (int row = 1; row < height; row++){

      // if the pixel is the leftmost pixel
      if (col == 0){
        forwardEnergy = energy(image, col, row, width, height);
        leftEnergy = energy(image, col + 1, row, width, height);
        totalEnergy += min(forwardEnergy, leftEnergy);
      }
      // if the pixel is the rightmost pixel
      else if (col == width - 1){
        forwardEnergy = energy(image, col, row, width, height);
        rightEnergy = energy(image, col - 1, row, width, height);
        totalEnergy += min(forwardEnergy, rightEnergy);
      }
      // if the pixel is not an edgecase pixel
      else{
        forwardEnergy = energy(image, col, row, width, height);
        leftEnergy = energy(image, col + 1, row, width, height);
        rightEnergy = energy(image, col - 1, row, width, height);
        totalEnergy += min(forwardEnergy, min(rightEnergy, leftEnergy));
      }
    }

    // set the first seam to the first row if the total energy is less than the minimum energy
    if (totalEnergy < minEnergy){
      minEnergy = totalEnergy;
      index = col;
    }
  }

  // load the minimum energy seam into the seam array
  loadVerticalSeam(image, index, width, height, seam);

  return seam;
}

int* findMinHorizontalSeam(const Pixel*const* image, int width, int height) {
  return nullptr;
}

void removeVerticalSeam(Pixel** image, int width, int height, int* verticalSeam) {
  // remove the vertical seam from the image
  Pixel temporary;
  
  for (int row = 0; row < height; row++){
    temporary = image[verticalSeam[row]][row];
    for (int col = verticalSeam[row]; col < width - 1; col++){
      image[col][row] = image[col + 1][row];
    }
    image[width - 1][row] = temporary;
  }
}

void removeHorizontalSeam(Pixel** image, int width, int height, int* horizontalSeam) {
  // remove the horizontal seam from the image
  Pixel temporary;

  for (int col = 0; col < width; col++){
    temporary = image[col][horizontalSeam[col]];
    for (int row = horizontalSeam[col]; row < height - 1; row++){
      image[col][row] = image[col][row + 1];
    }
    image[col][height - 1] = temporary;
  }
}