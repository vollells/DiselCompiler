#!/bin/bash

sudo docker login
sudo docker build --pull -t sjoelund/bionic-sphinx:`date +%Y%m%d` - < Dockerfile
sudo docker tag sjoelund/bionic-sphinx:`date +%Y%m%d` sjoelund/bionic-sphinx:latest
sudo docker push sjoelund/bionic-sphinx:`date +%Y%m%d`
sudo docker push sjoelund/bionic-sphinx:latest

