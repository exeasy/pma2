#include <utils/common.h>
#include "logger.h"
#include <utils/utils.h>
#include <utils/xml.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <unistd.h>
#include <lib/shmem.h>
#include <lib/semph.h>

#ifdef PTHREAD
pthread_mutex_t pthread_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif


struct logtype *typelist;
struct logtype *typeaddr[1024];
int type_total;

int memptr = 0;
int block_index=0;

extern int read_conf();
extern int parse_conf();

int log_type_init()
{
	typelist = (struct logtype*)malloc(sizeof( struct logtype));
	typelist->next = NULL;
	read_conf();
}

int log_type_add(const char* typename, int enabled)
{
	struct logtype *type = typelist;
	while(type->next!=NULL)
	{
		struct logtype *tmp = type->next;
		if(strcmp(tmp->name,typename) == 0)
			return -1;
		type = type->next;
	}
	struct logtype *temp = (struct logtype*)malloc(sizeof(struct logtype));
	temp->next =NULL;
	strcpy(temp->name,typename);
	temp->index = type_total;
	temp->flag = enabled;

	typeaddr[type_total++] = temp;

	type->next = temp;
	return 0;
}

struct logtype* get_logtype_addr(const char* name)
{
	for(int i = 0;i <type_total; i++)
	{
		if(typeaddr[i]->flag && strcmp(typeaddr[i]->name, name) == 0)
			return typeaddr[i];
	}
	return NULL;
}

int get_typeinfo(struct logmsg* msg)
{
#ifdef PTHREAD
	pthread_mutex_lock(&pthread_mutex);
#endif
	struct logtype *log_t = get_logtype_addr(msg->name);
	if(log_t != NULL)
	{
		gettimeofday(&msg->record_time,NULL);
		msg->last_time = log_t->last_time;
		msg->type = log_t->index;
		gettimeofday(&log_t->last_time,NULL);
	}
	else 
		msg->type = -1;
#ifdef PTHREAD
	pthread_mutex_unlock(&pthread_mutex);
#endif
}

int log_init()
{
	int shmid = create_shm("/tmp/pma.id", MAX_BLOCK * BLOCK_SIZE );
	set_shmqueue_length(shmid, MAX_BLOCK);
	printf("share memory create %d\n", shmid);
	memptr = shmid;
	int ret = init_sems("/tmp/pma.id", 3 );
	printf("semphore create %d\n", ret);
	if( ret == -1)
	{
		printf("semphore create failed\n");
		return -1;
	}
	else if( ret > 0)
	{
		// sem[0] lock
		ret = set_sem_value(MUTEX, 1);
		// 
		// sem[1] varys{0,MAX_BLOCK}
		ret = set_sem_value(EMPTY, MAX_BLOCK);
		// 
		// sem[2] varys{0,MAX_BLOCK}
		ret = set_sem_value(FULL, 0);
	}
	return 0;
}

int log_reset()
{
	log_init();
	log_close();
	log_init();
}


int logger(const char* name, const char* format, ...)
{
	va_list args;
	char *buff = (char*)calloc(MAX_BUFF_LEN,1);
	va_start(args,format);
	vfprintf(stdout,format,args);
	vsprintf(buff,format,args);
	va_end(args);

	unsigned int length = strlen(buff);

	struct logmsg *msg = (struct logmsg*)malloc( sizeof(struct logmsg) + length);
	strcpy(msg->name,  name);	
	get_typeinfo(msg);
	if(msg->type != -1)//type not found or disabled
	{
		msg->length = length;
		memcpy(msg->data, buff, length);

		free(buff);
		buff = NULL;
		p_sem(EMPTY);
		log_writer(msg);
		v_sem(FULL);
	}
	free(msg);
	msg = NULL;
}

int log_writer(struct logmsg* msg)
{
	//write
	p_sem(MUTEX);
	int head = get_shmqueue_head(memptr);
	int tail = get_shmqueue_tail(memptr);
	int size = get_shmqueue_length(memptr);
	if( (tail + 1)%size == head)
	{
		printf("Queue full\n");
		v_sem(MUTEX);
		return -1;
	}
	else{
		set_shmqueue_tail(memptr, (tail + 1)%size);
//		char * buff = log_package_json(msg);
		char * buff = log_package_xml(msg);
		//notice here 
		//if the data len > BLOCK_SIZE 
		//the left data will be lost
		write_shm(memptr, buff, tail*BLOCK_SIZE, strlen(buff+sizeof(int))+sizeof(int));
		printf("Block %d writed \n",tail);
		free(buff);
		v_sem(MUTEX);
		return 0;
	}
}

char* log_reader()
{
	p_sem(MUTEX);
	int head = get_shmqueue_head(memptr);
	int tail = get_shmqueue_tail(memptr);
	int size = get_shmqueue_length(memptr);
	if( head == tail )
	{
		printf("Queue Empty\n");
		v_sem(MUTEX);
		return NULL;
	}
	else 
	{
		set_shmqueue_head(memptr, (head + 1)%size);
		char* buff = (char*)malloc(BLOCK_SIZE);
		buff = read_shm(memptr,buff, head*BLOCK_SIZE, BLOCK_SIZE);
		printf("Block %d Readed\n",head);
		int size = *(int*)buff;
		char* realdata = (char*)malloc(size+1);
		memcpy(realdata, buff+sizeof(int), size);
		realdata[size] = 0;
		free(buff);
		printf("%d %s\n",size,realdata);
		v_sem(MUTEX);
		return realdata;
	}
}

char* log_package_xml(struct logmsg* msg)
{
	xmlDocPtr doc = create_xml_doc();
	xmlNodePtr devnode;
	xmlNodePtr childnode;
	
	xmlNodePtr rootnode = create_xml_root_node(doc, "event");
	struct timeval now = msg->record_time;
	char * time = PRINTTIME(now);
	add_xml_child(rootnode, "timestamp",time); free(time);

	char value[24];

	add_xml_child(rootnode, "name",msg->name); 
	memset(value, 0 , 24);
	sprintf(value,"%d", msg->type);
	add_xml_child(rootnode, "type", value);
	add_xml_child(rootnode, "data", msg->data);
	u8 *xmlbuff;
	int len  = 0;
	xmlDocDumpFormatMemoryEnc( doc, &xmlbuff, &len, "UTF-8", 0 );
	char *buff = (char*)malloc(len+1+sizeof(int));
	memcpy(buff+sizeof(int), xmlbuff, len);
	*(int*)buff = len;
	xmlFree(xmlbuff);
	xmlFreeDoc(doc);
	return buff;

}
char* log_package_json(struct logmsg* msg)
{
	char *buff = (char*)calloc(BLOCK_SIZE,1);
	char *t1 = PRINTTIME(msg->record_time);
	char *t2 = PRINTTIME(msg->last_time);
	sprintf(buff+sizeof(int), "{\"name\":\"%s\",\
\"type\":%d,\
\"recordtime\":\"%s\",\
\"lasttime\":\"%s\",\
\"len\":%d,\
\"data\":\"%s\"}",
			msg->name,msg->type,t1, t2, msg->length, msg->data);
	*(int*)buff = strlen(buff+sizeof(int));
	free(t1); free(t2);
	return buff;
}

int log_close()
{
	close_sems();
	close_shm(memptr);
}

int parase_line(char* line)
{
	char name[MAX_NAME_LEN];
	int flag = 0;
	if(line[0]=='#')return 0;//this line is comment
	char* tok = strtok(line,"=");
//	
//	if(strncmp(tok,"SERVER_IP",9)==0)
//	{
//		tok = strtok(NULL,"=");
//		strncpy(serverip,tok,strlen(tok));
//		printf("server ip read %s\n",serverip);
//		return 0;
//	}
//	else if(strncmp(tok,"SERVER_PORT",11)==0)
//	{
//		tok = strtok(NULL,"=");
//		int base = 10;
//		serverport = strtol(tok,NULL,base);
//		printf("server port read %d\n",serverport);
//		return 0;
//	}
	sprintf(name, "%s", tok);	
	printf("Type Name: %s\n",name);
	tok = strtok(NULL,"=");
	if(strncmp(tok,"true",4)==0||strncmp(tok,"TRUE",4)==0)
	flag = 1;
	else if(strncmp(tok,"false",5)==0||strncmp(tok,"FALSE",5)==0)
	flag = 0;
	log_type_add(name,flag);
	printf("Type State: %s\n",flag?"Enabled":"Disabled");
}
int parse_conf(FILE* fp)
{
	if(fp==NULL)return -1;
	fseek(fp,0,SEEK_SET);
	char linebuff[1024];
	while(!feof(fp))
	{
		char* p = fgets(linebuff,1024,fp);
		if(p==NULL)continue;
		if(-1 == parase_line(linebuff))continue;
	}
	return 0;
}


int read_conf()
{
	FILE *fp = fopen(CONFILE,"r+");
	if(fp==NULL)
		return -1;
	parse_conf(fp);
	fclose(fp);
}



