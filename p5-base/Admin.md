# Project Administration and Policies

* **Due Dates**: Unless otherwise communicated, projects will be released by Wednesday and due on Tuesday at midnight 2 weeks later.

* **Collaboration**: CS 537 has individual and team projects.  For individual projects, all work must be your own.  For team projects, all work must be completed by members of the team.  Copying code is considered cheating. [Read this](http://pages.cs.wisc.edu/~remzi/Classes/537/Spring2018/dontcheat.html) for more information on what is okay and what is not.  Please help us all have a good semester by not doing this.

* **Questions**: We will use [Piazza](https://piazza.com/class/mf19xjonaw03kw) for all project questions.

* **Environment**: All tests will be run from the [CS537-Docker container](https://git.doit.wisc.edu/cdis/cs/courses/cs537/useful-resources/cs537-docker) environment, which is a linux (Ubuntu 22.04) environment with development and additional tools installed.  The linux manual has not been included in this container.  We suggest using [Linux Man Pages Online](http://man.he.net/) to search the manual.

* **Tests**: A few sample tests are provided in the project repository.  To run them, from within the [CS537-Docker container](https://git.doit.wisc.edu/cdis/cs/courses/cs537/useful-resources/cs537-docker), go to the project's `tests/` directory and execute `run-tests.sh`.  Read the `README.md` file in that directory and try `run-tests.sh -h` to learn more about the testing script.  Note, these test cases are not complete, and you are encouraged to create more on your own.  **When grading, additional unreleased tests may be used.**

* **Submission**: To submit your work, you must push to the `main` branch of your project's repository on [DOIT's Gitlab instance](https://git.doit.wisc.edu/).

* **Late Days**: Projects will continue to be graded for three days past the due date and will not be accepted after the third day.  Project grades receive a 10 percentage point penalty for each day late.

* **Slip Days**:  You will have a bank of 2 slip days for the first 3 individual projects and 2 additional slip days for the 3 team projects.  These slip days will be automatically deducted from your bank rather than taking a penalty.  Once they are used, the late day penalty will be applied.  This all happens automatically so you do not need to ask for the slip days to be applied.

* **Grading**: The grading script will run one week after the project is released to provide test results on both released and hidden tests.  The grader runs inside the [CS537-Docker container](https://git.doit.wisc.edu/cdis/cs/courses/cs537/useful-resources/cs537-docker).  Grading results will be included in the `GRADING_SUMMARY.txt` file of the `grading` branch of your repository.  On the due date, the grading script will run again, and will run once a day for the next three days.  On the third day after the due date, the highest calculated grade will be transferred to Canvas.  For example, if, when it runs on the due date you receive a score of 70 / 100, and on the next day (1 day late) it runs and you also receive a score of 70 / 100 plus deducting 1 slip day, then it will transfer the 70 / 100 with no slip day deduction.  But if on the 1 day late run the grader calculates 71 / 100 plus deducting 1 slip day, then the 71 / 100 plus deducting 1 slip day will be transferred to Canvas.

* **Repository Structure**:  The solution folder of the `main` branch is what is pulled from your repository when running the grader.

* **LLM Usage**: For this course in particular we have seen these tools give close, but not quite right examples or explanations, that leave students more confused if they don't already know what the right answer is. Be aware that when you seek help from the instructional staff, we will not assist with working with these LLMs and we will expect you to be able to walk the instructional staff member through your code and logic.  **We do not recommend you use Large-Language Models such as ChatGPT**.

    - Online resources (e.g. stack overflow) and generative tools are transforming many industries including computer science and education.  However, if you use online sources, you are required to turn in a document, `resources.txt` in the root of your project repository describing your uses of these sources.
    - Indicate in this document what percentage of your solution was done strictly by you and what was done utilizing these tools. Be specific, indicating sources used and how you interacted with those sources.
    - Not giving credit to outside sources is a form of plagiarism. It can be good practice to make comments of sources in your code where that source was used.
    - You will not be penalized for using LLMs or reading posts, but you should not create posts in online forums about the projects in the course. The majority of your code should also be written from your own efforts and you should be able to explain all the code you submit.

* **Exceptions**: Any exceptions to these project policies will need to be requested from the instructor.

