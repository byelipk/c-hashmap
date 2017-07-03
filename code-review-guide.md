# How to perform a code review


* Start at the entry point for a piece of code that has changed.

* For each function, confirm that it's calling parameters are correct.

* Enter that function and confirm each line's correctness.

* When you encounter a function, repeat up to #2 until you go no further.

* As you exit functions, confirm their return values and their usage.

* Continue until you are back at where you started at the entry point.

* Do a diff on your changes and confirm any missed calls to changed functions.


##### Thanks to Zed Shaw @ Learn C The Hard Way for this approach!
