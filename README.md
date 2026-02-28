# Student management tool
A CLI program that allows to manage students inside the specific lecture. User can either create or load a program. Then user can enrol students and give them different properties, after that user can export lecture as a CSV file. Program supports arbitrary lenght input and one lecture can contain arbitrary amount of students. Documented in a Doxygen style.  

**Main focus areas:**  
- File I/O and CSV files
- Dynamic memory and arbitrary input
- Custom structs
- Valgrind
- Clean style and high maintainability

The program was tested with testcases, which allowed to cover worst-case scenarios and not just happy path.  

**A bit a of personal expirience:**  
It was fun to work with custom structs, as you shape them by yourself. CSV file was not a big hurdle to overcome, I rather had the most struggles with the limits and restrictions e.g. proper names for student and lecture, limits of the points, correctly written file. Of course a lot of time was spent with the memory managment, untlil the program became robust and leak-free.  

**Example of the program:**  
```
+===========================+
| Intelligent Study Program |
+===========================+

Please enter one of the following commands:
  create - create new lecture
  load   - load existing lecture
[] > create course2

Please enter one of the following commands:
  enrol  - enrol new student to the lecture
  remove - remove a student from the lecture
  give   - give points to a student
  calc   - calculate the grades for every student
  print  - print the lecture
  export - export the lecture to a file
  close  - close the lecture
[course2] > enrol studentA
[course2] > enrol studentB
[course2] > give 9 studentA
[course2] > give 2 studentB
[course2] > calc
[course2] > print
+===========================+
Lecture: course2
Number of students: 2
Average Grade: 3.00
+===========================+
Name: studentA
Points: 9
Grade: 1
+---------------------------+
Name: studentB
Points: 2
Grade: 5
+---------------------------+
[course2] > exit
Thank you for using the Intelligent Study Program!
```
