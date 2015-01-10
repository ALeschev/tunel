#define MAX_HASH 1024
#define RANDM 1987

typedef struct chain_el
{
    char name[MAX_NAME_ID];
	int type;
    struct chain_el *next;
} a;

a *ch_el_p;	
int count_h=1; 	//кол-во идентификаторов в таблице
a *hash_table[MAX_HASH];

void table_free_start();
void table_free(a *);

void table_init()
{
	int i;
	for (i=0;i<MAX_HASH;++i) hash_table[i]=NULL;
	ch_el_p=(a *)malloc(sizeof(a));
	ch_el_p->next=NULL;
}

int hash(const char *nameId)
{
	int res=0,i=0;
	for (i=0;i<strlen(nameId);res+=nameId[i],++i) ;
	res*=strlen(nameId);
//	res=(nameId[0]+nameId[nameId.length()-1])/nameId.length();
	res%=RANDM;
	return res;
}

a *table_add(char *name, int type)
{
	int n;
    a *anext;

	n=hash(name);
    anext=hash_table[n];
	if (anext==NULL)
	{   
		++count_h;
		hash_table[n]=anext=ch_el_p;
		strncpy(anext->name, name, MAX_NAME_ID);
		anext->type=type;
		anext->next=NULL;
        
		ch_el_p=(a *)malloc(sizeof(a));
		ch_el_p->next=NULL;
		return anext;
    }
	else
	{
		if (!strncmp(anext->name, name, MAX_NAME_ID))	return NULL;
		while (anext->next!=NULL)
		{
			if (!strncmp(anext->name, name, MAX_NAME_ID))	return NULL;
			anext=anext->next;
		}
		++count_h;
		anext->next=ch_el_p;
		strncpy(anext->next->name, name, MAX_NAME_ID);
		anext->type=type;
		anext->next->next=NULL;
                
		ch_el_p=(a *)malloc(sizeof(a));
		ch_el_p->next=NULL;
		return anext;
	}
}

a *table_find(char *name)
{
	int n;
	a *anext;
	
	n=hash(name);
	anext=hash_table[n];
	if (anext==NULL) return NULL;
	else if (!strncmp(anext->name, name, MAX_NAME_ID))	return anext;
	else
	{
		while (1)
		{
			if (anext->next!=NULL)
			{
				anext=anext->next;
				if (!strncmp(anext->name, name, MAX_NAME_ID))	return anext;
			}
		}
	}

	return NULL;
}

void table_free(a *id)
{
	if (id->next!=NULL)	table_free_start(id->next);
	free(id);
}

void table_free_start()
{
	count_h=1;
	int i=0;
    for (i=0;i<MAX_HASH;++i)
    {
		if (hash_table[i]!=NULL)
		{
			table_free(hash_table[i]);
			hash_table[i]=NULL;
		}
	}
}
