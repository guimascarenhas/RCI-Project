
	int fullpred=1, fullsucc=1, fullafd=1;
	 buffer1[128]="\0", buffer2[128]="\0", buffer3[128]="\0";





					if(state == busy && FD_ISSET(afd,&rfds)){
			//checks is the full message was received and puts it together if not
			readTCP(afd, buffer1, &fullafd);
			if(fullafd==1){
				//if the server has the full message it can handle it 
				if(messageHandler(afd, buffer1)){
					state=idle;
					write(1, "received: ",10);
					write(1, buffer1, strlen(buffer1));
					cleanBuffer(buffer1);
				}
			}
			else{
				close(afd);
				state = idle;
			}




				//reads input from successor
		else if(succ_fd>0 && FD_ISSET(succ_fd,&rfds)){
			//checks is the full message was received and puts it together if not
			readTCP(succ_fd, buffer2, &fullsucc);
			if(fullsucc==1){
				//if the server has the full message it can handle it 
				succMessageHandler(buffer2);
				write(1, "received from succ: ",20);
				write(1, buffer2, strlen(buffer2));
				cleanBuffer(buffer2);
			}
			else if(fullsucc==-1){
				succLeft();
				fullsucc=1;
			}
		}
		

		else if(pred_fd>0 && FD_ISSET(pred_fd,&rfds)){
			//checks is the full message was received and puts it together if not
			readTCP(pred_fd, buffer3, &fullpred);
			if(fullpred==1){
				//if the server has the full message it can handle it 
				predMessageHandler(buffer3);
				write(1, "received from pred: ",20);
				write(1, buffer3, strlen(buffer3));
				cleanBuffer(buffer3);
			}
		}


//sets the provided buffer to '\0'
void cleanBuffer(char* buff){
	for(int i=0; i<strlen(buff); i++){
		buff[i]='\0';
	} 
}

//reads the available content from a given fd 
void readTCP(int fd, char* buff, int* full){

	ssize_t n;
	char temp[128];
	int i=0;
	if((n=read(fd,temp,128))!=0){
		if(n==-1) exit(1);

		//if the last message was fully received or if its the first message, copies temp to buff
		if(*full==1){
			strcpy(buff,temp);
		}
		else{
			//if *full is different from 1 it means that the message wasnt received all together and we have to concatenate the strings
			strcat(buff, temp);
		}
		//checks if the message was fully received (search for '\n')
		for(i=0; i<strlen(temp); i++){
			if(strchr(temp, '\n')!=NULL){
				*full=1;
				return;
			}
		} 
		*full=0;
	}else *full=-1;	//a server has left 
}