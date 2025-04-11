# Shell Implementation

## הוראות הרצה
1. הקמפול את הפרויקט: gcc main.c shell_functions.c -o shell

2. הרץ את ה-shell:./shell

## תכונות נתמכות
* פקודות בסיסיות: cd, pwd, ls, cat, וכו'
* הפניית פלט: ls -l > output.txt
* צינורות: ls | grep txt
* פקודת tree: מציגה את מבנה הספרייה בפורמט עץ

## מבנה הקוד
* shell.h: הגדרות והצהרות פונקציות
* shell_functions.c: מימוש הפונקציות
* main.c: הלוגיקה הראשית של התוכנית
