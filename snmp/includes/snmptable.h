/*
 * snmptable.h
 *
 *  Created on: 2012-6-5
 *      Author: liupengzhan
 */

#ifndef SNMPTABLE_H_
#define SNMPTABLE_H_
int
snmp_table(char *ip,char *oidstr,char * comm,int version,int *entryptr,int *fieldptr,char ***dataptrr);

void
print_table(char **data,int entries, int fields);

void
free_data(char **datap,int e, int f);


#endif /* SNMPTABLE_H_ */
