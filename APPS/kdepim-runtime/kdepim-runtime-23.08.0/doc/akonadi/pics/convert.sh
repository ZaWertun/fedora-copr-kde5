#!/bin/bash

for i in Akonadi*.eps; do
  mv "$i" `echo $i | tr [A-Z] [a-z] | tr [\ ] [_]`;
done
