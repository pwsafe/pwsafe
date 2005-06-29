#!/bin/sh
# Classpath builder pinched from http://www.devdaily.com/blog/Content/2/17/404/
JAR_CLASSPATH=
for i in `ls *.jar`
do
  JAR_CLASSPATH=${JAR_CLASSPATH}:${i}
done
java -Djava.library.path=. -cp ${JAR_CLASSPATH} org.pwsafe.passwordsafeswt.PasswordSafeJFace
