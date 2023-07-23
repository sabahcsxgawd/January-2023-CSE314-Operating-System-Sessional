string=hello.world
echo ${string:2:1}
echo ${string:2:2}
echo ${string:2:99}
echo ${string: -1} # mind the space before - sign
echo ${string: -4}
echo ${string:0: -10}