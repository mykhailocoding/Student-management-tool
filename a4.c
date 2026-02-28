//---------------------------------------------------------------------------------------------------------------------
/// This program represents a grading tool for a class of students. Using different commands user can create or load
/// lecture and then manage it. The lecture consists of multiple students, each with a name, point total and optionally
/// a grade. User can enrol new students or remove them, give different amount of points to each including substracting
/// (with a minus sign), calculate grades for all student based on a highest score in the class and an average grade,
/// display a formatted summary of the lecture and all students, and export the current lecture as a csv file. Both
/// lectures and students are represented as structs and stored on the heap.
//---------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

typedef enum _Others_
{
  INITIAL_BUFFER_SIZE = 5,
  INCREASE_RATE_OF_THE_BUFFER_SIZE = 5
} Others;

typedef enum _Returns_ 
{
  QUIT = 27,
  LECTURE_CREATED = 0,
  FILE_LOADED = 0
} Returns;

typedef enum _Errors_
{
  MEMORY_ERROR = 300,
  UNKNOWN_COMMAND,
  WRONG_ARGUMENT,
  INCORRECT_LECTURE_NAME,
  FILE_ERROR,
  INCORRECT_STUDENTS_NAME,
  MALFORMED_ROW,
  UNSUCCESSFUL_LOAD,
  NOT_UNIQUE_NAME,
  STUDENT_NOT_FOUND,
  POINTS_LIMIT
} Errors;

typedef enum _Commands_ 
{ 
  CREATE = 90,
  LOAD,
  ENROL,
  REMOVE,
  GIVE,
  CALC,
  PRINT,
  EXPORT,
  CLOSE
} Commands;

typedef struct _Student_
{
  char* name_;
  int points_;
  int grade_;
} Student;

typedef struct _Lecture_
{
  char* name_;
  Student* students_;
  int amount_students_;
  float average_grade_;
} Lecture;

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function prints a welcome message.
void welcomeMessage(void)
{
  printf("+===========================+\n");
  printf("| Intelligent Study Program |\n");
  printf("+===========================+\n");
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function prints commands for a global mode.
void globalCommandsPrint(void)
{
  printf("\nPlease enter one of the following commands:\n");
  printf("  create - create new lecture\n");
  printf("  load   - load existing lecture\n");
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function finds end of the string and returns a pointer to it.
/// @param buffer initial string
/// @return pointer type char to the '\0' or NULL if it was not found
char* findEndOfTheString(char* buffer)
{
  for(char* iterator = buffer; *iterator != '\0'; ++iterator)
  {
    if(*iterator == '\n')
    {
      return iterator;
    }
  }
  return NULL;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function increases size of the buffer by a certain rate.
/// @param buffer initial string
/// @param buffer_size initial size of the buffer
/// @return buffer - pointer to the increased buffer, NULL if an error occured
char* increaseBufferSize(char* buffer, int* buffer_size)
{
  int new_buffer_size = *buffer_size + INCREASE_RATE_OF_THE_BUFFER_SIZE;
  char* temporal_buffer = realloc(buffer, new_buffer_size * sizeof(char));
  if(temporal_buffer == NULL)
  {
    return NULL;
  }
  buffer = temporal_buffer;//realloc automatically frees so we do not have to worry about that
  *buffer_size = new_buffer_size;
  return buffer;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function handles an arbitrary long user input. It starts with a fixed-size buffer then increases it
/// until all input has been read and stored on the heap using dynamic memory allocation.
/// Remark: readUserInput, increaseBufferSize and findEndOfTheString were taken from a3, as it has the same requirement
/// of the arbitrary input.
/// @return pointer to the allocated memory on the heap, NULL if an error occured
char* readUserInput(void)
{
  int buffer_size = INITIAL_BUFFER_SIZE;
  char* initial_buffer = malloc(buffer_size * sizeof(char));
  if(initial_buffer == NULL)
  {
    return NULL;
  }
  if(fgets(initial_buffer, buffer_size, stdin) == NULL)//If user presses EOF
  {
    free(initial_buffer);
    return NULL;
  }
  int offset = 1;                       //- 1 because of \0 
  while(findEndOfTheString(initial_buffer + offset - 1) == NULL)
  {                                     //+ offset - 1 to avoid checking parts that were already checked
    char* updated_buffer = increaseBufferSize(initial_buffer, &buffer_size);
    if(updated_buffer == NULL)
    {
      free(initial_buffer);
      return NULL;
    }
    initial_buffer = updated_buffer;
    offset = buffer_size - INCREASE_RATE_OF_THE_BUFFER_SIZE;
    if(fgets(initial_buffer + offset - 1, INCREASE_RATE_OF_THE_BUFFER_SIZE + 1, stdin) == NULL)
    {
      break;//if user somehow entered a string without '\n'
    }
  }
  char* newline = findEndOfTheString(initial_buffer);
  if(newline != NULL)
  {//we do this to change '\n' to '\0' and if there was EOF case there is no '\n'
    *newline = '\0';
  }
  return initial_buffer;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function identifies the command that the user has typed in and returns a value assigned to it. 
/// @param token_1 string to be identified
/// @return value of a certain command, or UNKNOWN_COMMAND if the command typed is unknown
int identifyCommand(char* token_1)
{
  if(token_1 == NULL)//to avoid segmentation crash with strcmp
  {
    return UNKNOWN_COMMAND;
  }
  if(strcmp(token_1, "exit") == 0)
  {
    return QUIT;
  }
  if(strcmp(token_1, "create") == 0)
  {
    return CREATE;
  }
  if(strcmp(token_1, "load") == 0)
  {
    return LOAD;
  }
  if(strcmp(token_1, "enrol") == 0)
  {
    return ENROL;
  }
  if(strcmp(token_1, "remove") == 0)
  {
    return REMOVE;
  }
  if(strcmp(token_1, "give") == 0)
  {
    return GIVE;
  }
  if(strcmp(token_1, "calc") == 0)
  {
    return CALC;
  }
  if(strcmp(token_1, "print") == 0)
  {
    return PRINT;
  }
  if(strcmp(token_1, "export") == 0)
  {
    return EXPORT;
  }
  if(strcmp(token_1, "close") == 0)
  {
    return CLOSE;
  }
  return UNKNOWN_COMMAND;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function tokenises input of the user, identifies the command, then checks the amount of the arguments
/// for each command.
/// @param input input of the user
/// @param token_1 first token
/// @param token_2 second token
/// @param token_3 third token
/// @return value of a command, QUIT if the user typed "exit", WRONG_ARGUMENT if command is unknown or wrong number of
/// arguments was used
int checkArgumentsGlobal(char** input, char** token_1, char** token_2, char** token_3)
{
  *token_1 = strtok(*input, " \t\v");
  *token_2 = strtok(NULL, " \t\v");
  *token_3 = strtok(NULL, " \t\v");
  int command = identifyCommand(*token_1);
  if(command == UNKNOWN_COMMAND)
  {
    printf("Error: Unknown command!\n");
    return WRONG_ARGUMENT;
  }
  if(command == QUIT && *token_2 == NULL)
  {
    return QUIT;
  }
  if(command == QUIT && *token_2 != NULL)
  {
    printf("Error: Invalid command usage!\n");
    return WRONG_ARGUMENT;
  }
  if(command != CREATE && command != LOAD)
  {
    printf("Error: This command cannot be used in the current mode!\n");
    return WRONG_ARGUMENT;
  }
  if(*token_2 == NULL)
  {
    printf("Error: Invalid command usage!\n");
    return WRONG_ARGUMENT;
  }
  if(*token_3 != NULL)
  {
    printf("Error: Invalid command usage!\n");
    return WRONG_ARGUMENT;
  }
  return command;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function checks the name of the lecture. Name is allowed to have letters and digits in it.
/// @param name string to be checked
/// @return 0 if name is valid, INCORRECT_LECTURE_NAME if name is invalid
int checkLectureName(char* name)
{
  for(int character_index = 0; *(name + character_index) != '\0'; character_index++)
  {
    if(isalnum(*(name + character_index)) == 0)
    {
      printf("Error: Name contains invalid characters!\n");
      return INCORRECT_LECTURE_NAME;
    }
  }
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function creates a lecture on the heap. It checks the name of the lecture, assigns it to the lecture
/// and initialises other lecture properties.
/// @param name name of the lecture
/// @param lecture pointer to the address of lecture on the heap(array of lectures)
/// @return 0 if success, INCORRECT_LECTURE_NAME if the name is invalid, MEMORY_ERROR if allocation failed
int createLecture(char* name, Lecture** lecture)
{
  if(checkLectureName(name) == INCORRECT_LECTURE_NAME)
  {
    return INCORRECT_LECTURE_NAME;
  }
  *lecture = (Lecture*)malloc(sizeof(Lecture));
  if(*lecture == NULL)
  {
    return MEMORY_ERROR;
  }
  (*lecture)->name_ = malloc(strlen(name) + 1);
  if((*lecture)->name_ == NULL)
  {
    free(*lecture);
    return MEMORY_ERROR;
  }
  strcpy((*lecture)->name_, name);
  (*lecture)->students_ = NULL;
  (*lecture)->amount_students_ = 0;
  (*lecture)->average_grade_ = 0;
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function finds the last slash in the path, then puts the null terminator to remove ".csv".
/// @param path string, from which we need to derive the name of the function
/// @return name of the lecture
char* getNameForLecture(char* path)
{
  bool slash = false;
  int path_length = strlen(path);
  int last_slash = 0;
  for(int character_index = path_length - 1; character_index >= 0 ; character_index--)
  {
    if(*(path + character_index) == '/')
    {
      last_slash = character_index;
      slash = true;
      break;
    }
  }
  int name_length = path_length - last_slash - 5;//.csv\0 - 5 characters
  if(slash)
  {
    *(path + last_slash + name_length + 1) = '\0';
    return path + last_slash + 1;
  }
  *(path + name_length + 1) = '\0';
  return path;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function calculates amount of students in the file.
/// @param file file
/// @return amount of students
int calculateAmountOfStudents(FILE* file)
{
  int amount_students = 0;
  int current_character = 0;
  while(current_character != EOF)
  {
    current_character = fgetc(file);
    if(current_character == '\n')
    {
      amount_students++;
    }
  }
  rewind(file);
  return amount_students;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function allocates an array of students on the heap in the lecture.
/// @param lecture lecture
/// @param amount_students amount of students
/// @return 0 if success, MEMORY_ERROR if allocation failed
int allocateStudents(Lecture* lecture, int amount_students)
{
  lecture->students_ = calloc(amount_students, sizeof(Student));
  if(lecture->students_ == NULL)
  {
    return MEMORY_ERROR;
  }
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function checks the name of the student. Name must be alphabetic.
/// @param students_name string to be checked
/// @return 0 if name is valid, INCORRECT_STUDENTS_NAME is name is invalid
int checkStudentsName(char* students_name)
{
  char current_character = 0;
  for(int character_index = 0; *(students_name + character_index) != '\0'; character_index++)
  {
    current_character = *(students_name + character_index);
    if(isalpha(current_character) == 0)
    {
      printf("Error: Name contains invalid characters!\n");
      return INCORRECT_STUDENTS_NAME;
    }
  }
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function takes an array of characters that contains points and other symbols. It distinguishes
/// characters that are relevant for the points and converts them to the number. It also checks whether the points are
/// in a range from 0 to 100. If there are any inappropriate characters it prints error message for a malformed row.
/// @param points_array array where point characters are awaited to be
/// @param path file path string, is used to print the error message 
/// @return number of points if success, MALFORMED_ROW if there are any inappropriate characters
int checkAndCalculatePoints(char points_array[], char* path)
{
  int index = 0;
  for(; index < 4; index++)
  {
    if(points_array[index] == ',' || points_array[index] == '\0')
    {
      break;
    }
  }//index is now max 4
  if(index == 0 || index >= 4)//if the ',' is first character or is not found/4th character
  {
    printf("Error: Invalid file: %s!\n", path);
    return MALFORMED_ROW;
  }
  points_array[index] = '\0';//at the max 3th position we assign a null terminator so that atoi works properly
  for(int current_index = 0; current_index < index; current_index++)
  {
    if(isdigit(points_array[current_index]) == 0)//we check whether the characters that we have are all numbers
    {
      printf("Error: Invalid file: %s!\n", path);
      return MALFORMED_ROW;
    }
  }
  int number_points = atoi(points_array);//finally we assemble a number from an array of digits
  if(number_points > 100 || number_points < 0)//check if it is in the allowed range
  {
    printf("Error: Invalid file: %s!\n", path);
    return MALFORMED_ROW;
  }
  return number_points;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function receives an array with characters that represent a grade. It checks whether these characters
/// are valid, derives a grade as a number from them and then checks whether the grade is in the right range.
/// @param grade_array array with characters that are responsible for the grade
/// @param path file path string, is used to print the error message 
/// @return grade if success, MALFORMED_ROW if row is malformed
int checkAndCalculateGrade(char grade_array[], char* path)
{
  if(grade_array[1] != '\n')
  {
    printf("Error: Invalid file: %s!\n", path);
    return MALFORMED_ROW;
  }
  if(isdigit(grade_array[0]) == 0)
  {
    printf("Error: Invalid file: %s!\n", path);
    return MALFORMED_ROW;
  }
  int grade = grade_array[0] - '0';
  if(grade > 5 || grade < 0)
  {
    printf("Error: Invalid file: %s!\n", path);
    return MALFORMED_ROW;
  }
  return grade;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function frees students.
/// @param lecture lecture where the students are
void freeStudents(Lecture* lecture)
{
  if(lecture->students_ == NULL)
  {
    return;
  }
  for(int student_index = 0; student_index < lecture->amount_students_; student_index++)
  {
    free((lecture->students_ + student_index)->name_);
    (lecture->students_ + student_index)->name_ = NULL;
  }
  free(lecture->students_);
  lecture->students_ = NULL;//to know that it's freed
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function frees the whole lecture.
/// @param lecture lecture
void freeLecture(Lecture* lecture)
{
  if(lecture == NULL)
  {
    return;
  }
  freeStudents(lecture);
  free(lecture->name_);
  lecture->name_ = NULL;
  free(lecture);
  lecture = NULL;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function reads information from the file, checks it and then assigns it to the lecture. It iterates
/// through all the rows in the file that represent students, gets information about the name, points and grade, checks
/// all this information with helper functions and assigns it to the current student.
/// @param file file, where the data is taken from
/// @param lecture lecture
/// @param amount_students amount of students
/// @param path path to that file, is used to print error message
/// @return 0 if success, MALFORMED_ROW if the data in file is invalid, MEMORY_ERROR if allocation failed
int writeFromFileToLecture(FILE* file, Lecture* lecture, int amount_students, char* path)
{
  int current_student = 0;
  while(current_student < amount_students)
  {
    int student_name_length = 0;
    int current_character = 0;
    while(current_character != ',')
    {
      current_character = fgetc(file);
      student_name_length++;//maybe change that
      if(current_character == EOF)
      {
        freeLecture(lecture);
        printf("Error: Invalid file: %s!\n", path);
        return MALFORMED_ROW;
      }//to avoid infinite loop if the file will end unexpectedly
    }//we have + 1 character for '\0' because it increases student_name_length when character is already ','
    //its one character longer but we need exactly that because we need one character for null terminator
    char* student_name = calloc(student_name_length, sizeof(char));
    if(student_name == NULL)
    {
      freeLecture(lecture);
      return MEMORY_ERROR;
    }
    fseek(file, -student_name_length, SEEK_CUR);
    fgets(student_name, student_name_length, file);
    if(checkStudentsName(student_name) == INCORRECT_STUDENTS_NAME)
    {
      freeLecture(lecture);
      free(student_name);
      return MALFORMED_ROW;
    }
    (lecture->students_ + current_student)->name_ = student_name;
    fseek(file, 1, SEEK_CUR);//skipping ','
    char points_array[4] = {0};//3 for numbers one for ','
    fgets(points_array, 4, file);
    int points = checkAndCalculatePoints(points_array, path);
    if(points == MALFORMED_ROW)
    {
      free(student_name);
      freeLecture(lecture);
      return MALFORMED_ROW;
    }
    (lecture->students_ + current_student)->points_ = points;
    if(points < 10)
    {
      fseek(file, -1, SEEK_CUR);
    }
    else if(points == 100)
    {
      fseek(file, 1, SEEK_CUR);
    }//now we are at the character after the ','
    char grade_array[3] = {0};
    fgets(grade_array, 3, file);
    int grade = checkAndCalculateGrade(grade_array, path);
    if(grade == MALFORMED_ROW)
    {
      free(student_name);
      freeLecture(lecture);
      return MALFORMED_ROW;
    }
    (lecture->students_ + current_student)->grade_ = grade;
    current_student++;
    lecture->amount_students_ = current_student;
  }
  lecture->amount_students_ = amount_students;
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function iterates through the students and checks whether there already is a student with a name that
/// is the same as the name of the target student. Remark: it starts with the student that is after the target one.
/// @param lecture lecture
/// @param current_student target student
/// @return 0 if the name is unique, NOT_UNIQUE_NAME if the name is not unique
int sameNamesExist(Lecture* lecture, int current_student)
{
  for(int student_index = current_student + 1; student_index < lecture->amount_students_; student_index++)
  {
    if(strcmp(lecture->students_[current_student].name_, lecture->students_[student_index].name_) == 0)
    {
      return NOT_UNIQUE_NAME;
    }
  }
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function checks if the lecture contains students with the same names.
/// @param lecture lecture
/// @return 0 if the names are unique, NOT_UNIQUE_NAME if not
int checkSameStudentNames(Lecture* lecture)
{
  for(int current_student = 0; current_student < lecture->amount_students_; current_student++)
  {
    if(sameNamesExist(lecture, current_student) == NOT_UNIQUE_NAME)
    {
      return NOT_UNIQUE_NAME;
    }
  }
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function represents command load. It opens a file, writes data from file to the lecture, closes file.
/// @param token_2 path to the file, gets modified at some point
/// @param lecture lecture
/// @param path copy of the second token, we need it to print error messages
/// @return 0 on success, FILE_ERROR if file has not opened, MEMORY_ERROR if allocation failed, 
/// INCORRECT_LECTURE_NAME if the name is invalid, MALFORMED_ROW if data in the file is invalid
int loadLecture(char* token_2, Lecture** lecture, char* path)
{
  FILE* file = fopen(token_2, "r");
  if(file == NULL)
  {
    printf("Error: Cannot open file: %s!\n", path);
    return FILE_ERROR;
  }
  char* lecture_name = getNameForLecture(token_2);
  int amount_students = calculateAmountOfStudents(file);
  int result = createLecture(lecture_name, lecture);
  if(result == MEMORY_ERROR)
  {
    fclose(file);
    return MEMORY_ERROR;
  }
  if(result == INCORRECT_LECTURE_NAME)
  {
    fclose(file);
    return INCORRECT_LECTURE_NAME;
  }
  if(allocateStudents(*lecture, amount_students) == MEMORY_ERROR)
  {
    fclose(file);
    return MEMORY_ERROR;
  }
  int writing_result = writeFromFileToLecture(file, *lecture, amount_students, path);
  if(writing_result == MEMORY_ERROR)
  {
    fclose(file);
    return MEMORY_ERROR;
  }
  if(writing_result == MALFORMED_ROW)
  {
    fclose(file);
    return MALFORMED_ROW;
  }
  if(checkSameStudentNames(*lecture) == NOT_UNIQUE_NAME)
  {
    fclose(file);
    freeStudents(*lecture);
    return MALFORMED_ROW;
  }
  fclose(file);
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function represents global mode. User is asked to type create/load with arguments. Then lecture is
/// created or loaded respectively.
/// @param lecture lecture
/// @param global_mode bool variable that is used to change modes
/// @return LECTURE_CREATED/FILE_LOADED on success, MEMORY_ERROR if allocation failed, QUIT if user typed "exit",
/// other values if something is wrong
int globalMode(Lecture** lecture, bool* global_mode)
{
  char* token_1;
  char* token_2;
  char* token_3;
  printf("[] > ");
  char* input = readUserInput();
  if(input == NULL)
  {
    return MEMORY_ERROR;
  }
  int command = checkArgumentsGlobal(&input, &token_1, &token_2, &token_3);
  if(command == QUIT)
  {
    free(input);
    return QUIT;
  }
  if(command == WRONG_ARGUMENT)
  {
    free(input);
    return WRONG_ARGUMENT;
  }
  if(command == CREATE)
  {
    int result = createLecture(token_2, lecture);
    if(result == INCORRECT_LECTURE_NAME)
    {
      free(input);
      return INCORRECT_LECTURE_NAME;//start again
    }
    if(result == MEMORY_ERROR)
    {
      free(input);
      return MEMORY_ERROR;//end program
    }
    free(input);
    *global_mode = false;
    return LECTURE_CREATED;//change mode in the loop
  }
  //if(command == LOAD)//because command atp can only be create or load we can remove this if
  char* path = malloc(strlen(token_2) + 1);
  if(path == NULL)
  {
    free(input);
    return MEMORY_ERROR;
  }
  strcpy(path, token_2);
  int load_result = loadLecture(token_2, lecture, path);
  if(load_result == MEMORY_ERROR)
  {
    free(path);
    free(input);
    return MEMORY_ERROR;//end program
  }
  if(load_result != 0)
  {
    free(path);
    free(input);
    return UNSUCCESSFUL_LOAD;//start again
  }
  free(path);
  free(input);
  *global_mode = false;
  return FILE_LOADED;//change mode in the loop
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function prints commands for a lecture mode.
void lectureCommandsPrint(void)
{
  printf("\nPlease enter one of the following commands:\n");
  printf("  enrol  - enrol new student to the lecture\n");
  printf("  remove - remove a student from the lecture\n");
  printf("  give   - give points to a student\n");
  printf("  calc   - calculate the grades for every student\n");
  printf("  print  - print the lecture\n");
  printf("  export - export the lecture to a file\n");
  printf("  close  - close the lecture\n");
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function checks number of arguments for each command in the lecture mode.
/// @param command value of a command
/// @param token_2 second argument
/// @param token_3 third argument
/// @param token_4 fourth argument
/// @return 0 on success, WRONG_ARGUMENT if number of the arguments for a specific function is wrong
int checkNumberArgumentsLecture(int command, char* token_2, char* token_3, char* token_4)
{
  if(command == ENROL || command == REMOVE)//1 parameter(student name)
  {
    if(token_3 != NULL)
    {
      printf("Error: Invalid command usage!\n");
      return WRONG_ARGUMENT;
    }
    if(token_2 == NULL)
    {
      printf("Error: Invalid command usage!\n");
      return WRONG_ARGUMENT;
    }
  }
  if(command == GIVE)//2 parameters(points, student name)
  {
    if(token_4 != NULL)
    {
      printf("Error: Invalid command usage!\n");
      return WRONG_ARGUMENT;
    }
    if(token_3 == NULL)
    {
      printf("Error: Invalid command usage!\n");
      return WRONG_ARGUMENT;
    }
  }
  if(command == CALC || command == PRINT || command == EXPORT || command == CLOSE)//no parameters
  {
    if(token_2 != NULL)
    {
      printf("Error: Invalid command usage!\n");
      return WRONG_ARGUMENT;
    }
  }
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function tokenises input of the user, identifies value of the command and checks number of arguments.
/// @param input user input
/// @param token_1 command
/// @param token_2 second argument
/// @param token_3 third argument
/// @param token_4 fourth argument
/// @return command on success, QUIT if user typed "exit", WRONG_ARGUMENT if number of the arguments is wrong
int getAndCheckArgumentsLecture(char** input, char** token_1, char** token_2, char** token_3, char** token_4)
{
  *token_1 = strtok(*input, " \t\v");
  *token_2 = strtok(NULL, " \t\v");
  *token_3 = strtok(NULL, " \t\v");
  *token_4 = strtok(NULL, " \t\v");
  int command = identifyCommand(*token_1);
  if(command == QUIT && *token_2 ==  NULL)
  {
    return QUIT;
  }
  if(command == QUIT && *token_2 !=  NULL)
  {
    printf("Error: Invalid command usage!\n");
    return WRONG_ARGUMENT;
  }
  if(command == UNKNOWN_COMMAND)
  {
    printf("Error: Unknown command!\n");
    return WRONG_ARGUMENT;
  }
  if(command == CREATE || command == LOAD)
  {
    printf("Error: This command cannot be used in the current mode!\n");
    return WRONG_ARGUMENT;
  }
  if(checkNumberArgumentsLecture(command, *token_2, *token_3, *token_4) == WRONG_ARGUMENT)
  {
    return WRONG_ARGUMENT;
  }
  return command;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function searches for a student in the lecture using the name of the student.
/// @param lecture lecture
/// @param name name of the target student
/// @return index of the target student in the students array of the lecture on success, STUDENT_NOT_FOUND on failure
int studentNameInLecture(Lecture* lecture, char* name)
{
  for(int student_index = 0; student_index < lecture->amount_students_; student_index++)
  {
    if(strcmp(name, (lecture->students_ + student_index)->name_) == 0)
    {
      return student_index;
    }
  }
  return STUDENT_NOT_FOUND;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function represents a command enrol. It checks the name of the student and whether there already is a
/// student with such a name. Then it reallocates array of students, adds a new student there and initialises points 
/// and grade of that student.
/// @param lecture lecture
/// @param name name of the new student
/// @return 0 on success, MEMORY_ERROR if (re)allocation failed, other values if name is invalid or name is not unique
int enrol(Lecture* lecture, char* name)
{
  if(checkStudentsName(name) == INCORRECT_STUDENTS_NAME)
  {
    return INCORRECT_STUDENTS_NAME;
  }
  if(studentNameInLecture(lecture, name) != STUDENT_NOT_FOUND)
  {
    printf("Error: Student already exists, please enter another name!\n");
    return NOT_UNIQUE_NAME;
  }
  lecture->amount_students_++;
  lecture->students_ = realloc(lecture->students_, lecture->amount_students_ * sizeof(Student));
  if(lecture->students_ == NULL)
  {
    return MEMORY_ERROR;
  }
  int name_length = strlen(name) + 1;//+1 for \0
  (lecture->students_ + lecture->amount_students_ - 1)->name_ = malloc(name_length * sizeof(char));
  if((lecture->students_ + lecture->amount_students_ - 1)->name_ == NULL)
  {
    return MEMORY_ERROR;
  }
  strcpy((lecture->students_ + lecture->amount_students_ - 1)->name_, name);// -1 because index
  (lecture->students_ + lecture->amount_students_ - 1)->points_ = 0;//we need to do that because realloc gives
  (lecture->students_ + lecture->amount_students_ - 1)->grade_ = 0;// us new memory with random values in it
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function deletes grades of all students and average grade.
/// @param lecture lecture
void deleteGradesAndAverage(Lecture* lecture)
{
  for(int student_index = 0; student_index < lecture->amount_students_; student_index++)
  {
    (lecture->students_ + student_index)->grade_ = 0;
  }
  lecture->average_grade_ = 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This program "deletes" target student by shifting all of the students after that student to the left by 1
/// position.
/// @param lecture lecture
/// @param student_index index of the target student
void moveStudents(Lecture* lecture, int student_index)
{
  free((lecture->students_ + student_index)->name_);
  for(; student_index < lecture->amount_students_ - 1; student_index++)
  {
    (lecture->students_ + student_index)->name_ = (lecture->students_ + student_index + 1)->name_;
    (lecture->students_ + student_index)->grade_ = (lecture->students_ + student_index + 1)->grade_;
    (lecture->students_ + student_index)->points_ = (lecture->students_ + student_index + 1)->points_;
  }
  (lecture->students_ + student_index)->name_ = NULL;//no + 1 because it is now = lecture->amount_students_ - 1
  lecture->amount_students_--;                       //which is exactly last student
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function represents a command remove. It finds a target student, deletes all grades and average grade,
/// removes target student, reallocates array of the students to free unused memory.
/// @param lecture lecture
/// @param name name of the target student
/// @return 0 on success, STUDENT_NOT_FOUND on failure, MEMORY_ERROR if realloc fails
int removeStudent(Lecture* lecture, char* name)
{
  int student_index = studentNameInLecture(lecture, name);
  if(student_index == STUDENT_NOT_FOUND)
  {
    printf("Error: Student not found!\n");
    return STUDENT_NOT_FOUND;
  }
  deleteGradesAndAverage(lecture);
  moveStudents(lecture, student_index);
  if(lecture->amount_students_ == 0)
  {
    free(lecture->students_);
    lecture->students_ = NULL;
    return 0;
  }
  lecture->students_ = realloc(lecture->students_, lecture->amount_students_ * sizeof(Student));
  if(lecture->students_ == NULL)//no need for temporary pointer because we exit if allocation has failed
  {
    return MEMORY_ERROR;
  }
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function receives an array of characters, that represent points. This array can also have '-' to
/// substract points. Then function checks whether we need to add or substract them, checks whether they are valid 
/// characters and converts them to an integer.
/// @param points array that represent points
/// @param add logical variable to add/substract
/// @return number points on success, WRONG_ARGUMENT on failure
int extractAndCheckPoints(char* points, bool* add)
{
  int points_number = 0;
  int points_length = strlen(points);
  if(*points == '-')
  {
    *add = false;//substract points
    if(points_length > 4)
    {
      return WRONG_ARGUMENT;
    }
    for(int points_index = 1 ; points_index < points_length; points_index++)
    {
      if(isdigit(*(points + points_index)) == 0)
      {
        return WRONG_ARGUMENT;
      }
    }
    points_number = atoi(points + 1);
    if(points_number > 100 || points_number < 0)
    {
      return WRONG_ARGUMENT;
    }
    return points_number;
  }
  *add = true;//add points
  if(points_length > 3)
  {
    return WRONG_ARGUMENT;
  }
  for(int points_index = 0 ; points_index < points_length; points_index++)
  {
    if(isdigit(*(points + points_index)) == 0)
    {
      return WRONG_ARGUMENT;
    }
  }
  points_number = atoi(points);
  if(points_number > 100 || points_number < 0)
  {
    return WRONG_ARGUMENT;
  }
  return points_number;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function checks whether the operation with points will cause points of a target student to exceed the
/// limit.
/// @param lecture lecture
/// @param student_index target student
/// @param points number of points
/// @param add logical variable to add/substract
/// @return number points on success, POINTS_LIMIT on failure
int pointsLimit(Lecture* lecture, int student_index, int points, bool add)
{
  if((lecture->students_ + student_index)->points_ + points > 100 && add)
  {
    return POINTS_LIMIT;
  }
  if((lecture->students_ + student_index)->points_ - points < 0 && !add)
  {
    return POINTS_LIMIT;
  }
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function adds/substracts certain amount of points to/from a target student.  
/// @param lecture lecture
/// @param points number of points to add/substract
/// @param student_index target student
/// @param add logical variable to add/substract
void givePointsToStudent(Lecture* lecture, int points, int student_index, bool add)
{
  if(add)
  {
    (lecture->students_ + student_index)->points_ += points;
    return;
  }
  if(!add)
  {
    (lecture->students_ + student_index)->points_ -= points;
    return;
  }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This fucntion represents a command give. It extracts points from an argument, finds a target student, 
/// checks the points limit, deletes grades and average and gives/substracts points to/from the target student.
/// @param lecture lecture
/// @param points argument which is responsible for points
/// @param name name of the target student
/// @return 0 on success, WRONG_ARGUMENT on failure
int give(Lecture* lecture, char* points, char* name)
{
  bool add =  true;
  int points_number = extractAndCheckPoints(points, &add);
  if(points_number == WRONG_ARGUMENT)
  {
    printf("Error: Invalid command usage!\n");
    return WRONG_ARGUMENT;
  }
  int student_index = studentNameInLecture(lecture, name);
  if(student_index == STUDENT_NOT_FOUND)
  {
    printf("Error: Student not found!\n");
    return WRONG_ARGUMENT;
  }
  if(pointsLimit(lecture, student_index, points_number, add) == POINTS_LIMIT)
  {
    printf("Error: Points limit exceeded!\n");
    return WRONG_ARGUMENT;
  }
  deleteGradesAndAverage(lecture);
  givePointsToStudent(lecture, points_number, student_index, add);
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function iterates through all the students and finds the highest amount of points among them.
/// @param lecture lecture
/// @return highest amount of points
int findHighestPoints(Lecture* lecture)
{
  int highest_points = 0;
  for(int student_index = 0; student_index < lecture->amount_students_; student_index++)
  {
    if((lecture->students_ + student_index)->points_ > highest_points)
    {
      highest_points = (lecture->students_ + student_index)->points_;
    }
  }
  return highest_points;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function calculates average grade in the lecture.
/// @param lecture lecture
/// @return average grade
float calculateAverageGrade(Lecture* lecture)
{
  float total = 0;
  for(int student_index = 0; student_index < lecture->amount_students_; student_index++)
  {
    total += (lecture->students_ + student_index)->grade_;
  }
  float average_grade = total / lecture->amount_students_;
  return average_grade;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function assigns a grade to each student according to the highest number of points in the lecture.
/// @param lecture lecture
void calc(Lecture* lecture)
{
  if(lecture->amount_students_ == 0)
  {
    return;
  }
  int highest_points = findHighestPoints(lecture);
  if(highest_points == 0)
  {
    for(int student_index = 0; student_index < lecture->amount_students_; student_index++)
    {
      (lecture->students_ + student_index)->grade_ = 1;
    }
    lecture->average_grade_ = 1;
    return;
  }
  for(int student_index = 0; student_index < lecture->amount_students_; student_index++)
  {
    int percentage = (lecture->students_ + student_index)->points_ * 100 / highest_points;
    if(percentage >= 87)
    {
      (lecture->students_ + student_index)->grade_ = 1;
      continue;
    }
    if(percentage >= 75)
    {
      (lecture->students_ + student_index)->grade_ = 2;
      continue;
    }
    if(percentage >= 62)
    {
      (lecture->students_ + student_index)->grade_ = 3;
      continue;
    }
    if(percentage >= 51)
    {
      (lecture->students_ + student_index)->grade_ = 4;
      continue;
    }
    if(percentage < 51)
    {
      (lecture->students_ + student_index)->grade_ = 5;
      continue;
    }
  }
  lecture->average_grade_ = calculateAverageGrade(lecture);
  return;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function prints average grade and students with their grades.
/// @param lecture lecture
void printWithGrades(Lecture* lecture)
{
  printf("Average Grade: %.2f\n", lecture->average_grade_);
  printf("+===========================+\n");
  for(int student_index = 0; student_index < lecture->amount_students_; student_index++)
  {
    printf("Name: %s\n", (lecture->students_ + student_index)->name_);
    printf("Points: %d\n", (lecture->students_ + student_index)->points_);
    printf("Grade: %d\n", (lecture->students_ + student_index)->grade_);
    printf("+---------------------------+\n");
  }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function prints students without their grades.
/// @param lecture lecture
void printWithoutGrades(Lecture* lecture)
{
  printf("+===========================+\n");
  for(int student_index = 0; student_index < lecture->amount_students_; student_index++)
  {
    printf("Name: %s\n", (lecture->students_ + student_index)->name_);
    printf("Points: %d\n", (lecture->students_ + student_index)->points_);
    printf("+---------------------------+\n");
  }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function prints students with their grades but without an average grade of the lecture.
/// @param lecture lecture
void printWithoutAverage(Lecture* lecture)
{
  printf("+===========================+\n");
  for(int student_index = 0; student_index < lecture->amount_students_; student_index++)
  {
    printf("Name: %s\n", (lecture->students_ + student_index)->name_);
    printf("Points: %d\n", (lecture->students_ + student_index)->points_);
    printf("Grade: %d\n", (lecture->students_ + student_index)->grade_);
    printf("+---------------------------+\n");
  }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function is responsible for printing lecture info. Depending on whether there is an average grade and
/// whether students have their grades as well or not it prints grades and average, just grades or it does not print
/// grades and average at all.
/// @param lecture lecture
void print(Lecture* lecture)
{
  printf("+===========================+\n");
  printf("Lecture: %s\n", lecture->name_);
  printf("Number of students: %d\n", lecture->amount_students_);
  if(lecture->amount_students_ != 0 && lecture->average_grade_ == 0.0f && (lecture->students_)->grade_ != 0)
  {
    printWithoutAverage(lecture);
    return;
  }
  if(lecture->average_grade_ == 0.0f)
  {
    printWithoutGrades(lecture);
    return;
  }
  printWithGrades(lecture);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function writes data from the lecture to the csv file, which it creates. It iterates through all the
/// students and write them down as rows in csv file.
/// @param lecture lecture
/// @return 0 on success, FILE_ERROR if file could not be created, MEMORY_ERROR if alocation failed
int export(Lecture* lecture)
{
  int lecture_name_length = strlen(lecture->name_);
  char* file_path = malloc(13 + lecture_name_length * sizeof(char));//reports\\.csv - 12 characters + \0
  if(file_path == NULL)
  {
    return MEMORY_ERROR;
  }
  sprintf(file_path, "reports/%s.csv", lecture->name_);//printf, but in string
  FILE* file = fopen(file_path, "w");
  free(file_path);
  if(file == NULL)
  {
    printf("Error: Report could not be created!\n");
    return FILE_ERROR;
  }
  for(int student_index = 0; student_index < lecture->amount_students_; student_index++)
  {
    fprintf(file, "%s", (lecture->students_ + student_index)->name_);
    fprintf(file, ",");
    fprintf(file, "%d", (lecture->students_ + student_index)->points_);
    fprintf(file, ",");
    fprintf(file, "%d", (lecture->students_ + student_index)->grade_);
    fprintf(file, "\n");
  }
  fclose(file);
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function represents a command close. It frees the lecture and changes mode to the global.
/// @param lecture lecture
/// @param global_mode logical variable, represents a global or lecture mode
void close(Lecture* lecture, bool* global_mode)
{
  freeLecture(lecture);
  *global_mode = true;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function handles the logic of the lecture mode. It executes one of the commands and then takes care of
/// the returns of the command.
/// @param lecture lecture
/// @param token_2 second argument
/// @param token_3 third argument
/// @param global_mode logical variable, represents a global or lecture mode
/// @param command command(first argument)
/// @return 0 on success, WRONG_ARGUMENT if argument usage is invalid, MEMORY_ERROR if allocation failed
int lectureCommandsExecution(Lecture* lecture, char* token_2, char* token_3, bool* global_mode, int command)
{
  if(command == ENROL)
  {
    int result = enrol(lecture, token_2);
    if(result == MEMORY_ERROR)
    {
      return MEMORY_ERROR;
    }
    if(result != 0)//any not memory problem
    {
      return WRONG_ARGUMENT;
    }
  }
  if(command == REMOVE)
  {
    int result = removeStudent(lecture, token_2);
    if(result == MEMORY_ERROR)
    {
      return MEMORY_ERROR;
    }
    if(result == STUDENT_NOT_FOUND)
    {
      return WRONG_ARGUMENT;
    }
  }
  if(command == GIVE)
  {
    if(give(lecture, token_2, token_3) == WRONG_ARGUMENT)
    {
      return WRONG_ARGUMENT;
    }
  }
  if(command == CALC)
  {
    calc(lecture);
  }
  if(command == PRINT)
  {
    print(lecture);
  }
  if(command == EXPORT)
  {
    int result = export(lecture);
    if(result == MEMORY_ERROR)
    {
      return MEMORY_ERROR;
    }
    if(result == FILE_ERROR)
    {
      return WRONG_ARGUMENT;
    }
  }
  if(command == CLOSE)
  {
    close(lecture, global_mode);
  }
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function represents a lecture mode. It asks user for the input, checks whether the command and its
/// arguments are correct, executes the command and frees the input.
/// @param lecture lecture
/// @param global_mode logical variable, represents a global or lecture mode
/// @return 0 on success, WRONG_ARGUMENT if argument usage is invalid, MEMORY_ERROR if allocation failed
int lectureMode(Lecture* lecture, bool* global_mode)
{
  char* token_1;
  char* token_2;
  char* token_3;
  char* token_4;
  printf("[%s] > ", lecture->name_);
  char* input = readUserInput();
  if(input == NULL)
  {
    return MEMORY_ERROR;
  }
  int command = getAndCheckArgumentsLecture(&input, &token_1, &token_2, &token_3, &token_4);
  if(command == QUIT)
  {
    free(input);
    return QUIT;
  }
  if(command == WRONG_ARGUMENT)
  {
    free(input);
    return WRONG_ARGUMENT;
  }
  int result = lectureCommandsExecution(lecture, token_2, token_3, global_mode, command);
  if(result == MEMORY_ERROR)
  {
    free(input);
    return MEMORY_ERROR;
  }
  if(result == WRONG_ARGUMENT)
  {
    free(input);
    return WRONG_ARGUMENT;
  }
  free(input);
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief This function is responsible for the workflow of the main. 
/// @param global_mode logical variable, represents a global or lecture mode
/// @param lecture lecture
/// @param run logical variable, tells whether the program runs or ends
/// @return 0 on success, QUIT if user typed "exit", MEMORY_ERROR on memory error
int flow(bool* global_mode, Lecture** lecture, bool* run)
{
  if(*global_mode)
  {
    *lecture = NULL;//because of dangling pointer
    int result = globalMode(lecture, global_mode);
    if(result == MEMORY_ERROR)
    {
      return MEMORY_ERROR;
    }
    if(result == QUIT)
    {
      *run = false;
      return QUIT;
    }
    if(*global_mode == false)
    {//we print commands only when the mode changes
      lectureCommandsPrint();
    }
  }
  if(*global_mode == false)
  {
    int result = lectureMode(*lecture, global_mode);
    if(result == MEMORY_ERROR)
    {
      return MEMORY_ERROR;
    }
    if(result == QUIT)
    {
      *run = false;
      return QUIT;
    }
    if(*global_mode == true)
    {//we print commands only when the mode changes
      globalCommandsPrint();
    }
  }
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief Stuff happens here! User is greeted, then commands are printed and the program enters loop with a workflow.
/// In the end a farewell message is printed.
/// @return 0 - program terminated successfully, 1 - program was not able to allocate new memory
int main(void)
{
  bool run = true;
  bool global_mode = true;
  welcomeMessage();
  globalCommandsPrint();
  Lecture* lecture = NULL;
  while(run)
  {
    if(flow(&global_mode, &lecture, &run) == MEMORY_ERROR)
    {
      printf("Error: Out of memory!\n");
      freeLecture(lecture);
      return 1;
    }
  }
  freeLecture(lecture);
  printf("Thank you for using the Intelligent Study Program!\n");
  return 0;
}
