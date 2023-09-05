#include "rqsvr.h"

static	int _cmp_symb();

static	pthread_mutex_t mutex4symb;
static	int modified;


/*
 * init_board()
 */
int init_board()
{
	pthread_mutex_init(&mutex4symb, NULL);
	symbol_file_to_board();
	return(0);
}


/*
 * insert_symbol_to_board()
 * Insert symbol to RQ/Board
 */
int insert_symbol_to_board(char *symbcode, struct rq_client *client, mask_t mask)
{
	struct	symb *symb, symbol;
	int	newsymb = 0;
	int	indx, room;

	memset(&symbol, 0, sizeof(symbol));
	strcpy(symbol.symb, symbcode);

#ifdef	DEBUG
	dolog(__func__, LL_DEBUG, "symbol=%s, total symbols=%d/%d", symbol.symb, rt_symbol->valr, rt_symbol->maxr);
#endif
	pthread_mutex_lock(&mutex4symb);
	if ((symb = (struct symb *)bsearch(&symbol, rt_symbol->symb, rt_symbol->valr, sizeof(struct symb), _cmp_symb)) != NULL)
	{
		room = symb->seqn;
		if (!symb->mark)
		{
			// deleted symbol
			// marking to valid symbol
			rt_board->hook[room].many = 0;			// nobody listening
			rt_board->hook[room].mark = 1;			// set valid mark
			symb->mark = 1;					// deleted -> valid(used)
			modified++;					// modified symbol
		}
		if (!rt_board->hook[room].many)
			newsymb = 1;
		if (client != NULL)
		{
			switch (client->mask[room])
			{
			case 0: client->mask[room]  = mask;		// new symbol for a specified client
				rt_board->hook[room].many++;
				break;
			default:client->mask[room] |= mask;		// add mask for a exist symbol 
				break;
			}
			client->chck[room] |= mask;			// add latest mask information
		}
#ifdef	DEBUG
		dolog(__func__, LL_DEBUG, "symbol=%s checked at room=%d", symbcode, symb->seqn);
#endif
		pthread_mutex_unlock(&mutex4symb);
		return(symb->seqn);
	}
	if ((room = rt_symbol->valr) >= rt_symbol->maxr)
		cleaning_board(&indx, &room);
	else
	{
		room = rt_symbol->valr;
		indx = room;
	}
#ifdef	DEBUG
	dolog(__func__, LL_DEBUG, "symbol=%s allocated %d/%d", symbcode, room, rt_symbol->maxr);
#endif
	if (room < 0 || room >= no_of_symbols)
	{
		pthread_mutex_unlock(&mutex4symb);
		return(-1);
	}
	symbol.seqn = room;
	memcpy(&rt_symbol->symb[indx], &symbol, sizeof(symbol));
	if (indx >= rt_symbol->valr)
		rt_symbol->valr = indx + 1;
	qsort(rt_symbol->symb, rt_symbol->valr, sizeof(rt_symbol->symb[0]), _cmp_symb);
	symb = (struct symb *)bsearch(&symbol, rt_symbol->symb, rt_symbol->valr, sizeof(struct symb), _cmp_symb);
	if (symb != NULL)
	{
		symb->mark = 1;
		rt_board->hook[symb->seqn].many = 0;
		strcpy(rt_board->hook[symb->seqn].symb, symb->symb);
		rt_board->hook[symb->seqn].mark = 1;
	}
	if (client != NULL)
	{
		switch (client->mask[room])
		{
		case 0: client->mask[room]  = mask;
			rt_board->hook[room].many++;
			break;
		default:client->mask[room] |= mask;
			break;
		}
		client->chck[room] |= mask;
	}
	
	pthread_mutex_unlock(&mutex4symb);
	return(room);
}

/*
 * delete_symbol_from_board()
 * Delete RQ symbol
 */
int delete_symbol_from_board(char *symbcode)
{
	struct	symb *symb, symbol;
	int	seqn;

	memset(&symbol, 0, sizeof(symbol));
	strcpy(symbol.symb, symbcode);
	pthread_mutex_lock(&mutex4symb);
	symb = (struct symb *)bsearch(&symbol, rt_symbol->symb, rt_symbol->valr, sizeof(struct symb), _cmp_symb);
	if (symb != NULL && symb->mark)
	{
		modified++;
		symb->mark = 0;
               if ((seqn = symb->seqn) >= 0 && seqn < no_of_symbols)
                        rt_board->hook[seqn].mark = 0;
	}
	pthread_mutex_unlock(&mutex4symb);
	return(0);
}

/*
 * reset_symbol_on_board()
 * Reset (Delete all) symbols
 */
int reset_symbol_on_board()
{
	pthread_mutex_lock(&mutex4symb);
	memset(rt_board->hook, 0, no_of_symbols * sizeof(rt_board->hook[0]));
	rt_symbol->valr = 0;
	pthread_mutex_unlock(&mutex4symb);
	modified++;
	return(0);
}

/*
 * cleaning_board()
 * Cleaning RQ symbol board
 */
int cleaning_board(int *indx, int *seqn)
{
	int	ii, jj, kk;

	*indx = -1;
	*seqn = -1;
	if (!dynamic_mode)
		return(0);
	for (ii = 0, kk = -1; ii < rt_symbol->maxr; ii++)
	{
		if ((jj = rt_symbol->symb[ii].seqn) < 0 || jj >= rt_symbol->maxr)
			continue;
		if (rt_board->hook[jj].many <= 0)
		{
			rt_board->hook[ii].mark = 0;
			rt_symbol->symb[ii].mark = 0;		// marking to deleted symbol
			if (*indx == -1)
			{
				*indx = ii;
				*seqn = jj;
			}
		}
	}
	return(0);
}

/*
 * search_symbol_on_board()
 * Search Board
 */
int search_symbol_on_board(char *symbcode)
{
	struct	symb *symb, symbol;

	memset(&symbol, 0, sizeof(symbol));
	strcpy(symbol.symb, symbcode);
	symb = (struct symb *)bsearch(&symbol, rt_symbol->symb, rt_symbol->valr, sizeof(struct symb), _cmp_symb);
	if (symb == NULL || !symb->mark)
		return(-1);
	return(symb->seqn);
}

/*
 * board_symbol_to_file()
 * Save symbols to log file
 */
int board_symbol_to_file()
{
	FILE	*symbF;
	int	ii;

	if (dynamic_mode || strlen(symbol_file) <= 0 || !modified)
		return(0);
	if ((symbF = fopen(symbol_file, "w")) == NULL)
		return(0);

	modified = 0;
	for (ii = 0; ii < rt_symbol->valr; ii++)
	{
		if (!rt_symbol->symb[ii].mark)
			continue;
		fprintf(symbF, "%s\n", rt_symbol->symb[ii].symb);
	}
	fclose(symbF);
	return(0);
}

/*
 * symbol_file_to_board()
 * Save symbols to log file
 */
int symbol_file_to_board()
{
	char	line_b[512], symb_b[128];
	FILE	*symbF;
	int	ii;

	if (dynamic_mode || strlen(symbol_file) <= 0)
		return(0);
	if ((symbF = fopen(symbol_file, "r")) == NULL)
		return(0);

	ii = 0;
	while ((fgets(line_b, sizeof(line_b), symbF)) == line_b)
	{
		symb_b[0] = '\0';
		sscanf(line_b, "%s", symb_b);
		if (!(strlen(symb_b) > 0 && strlen(symb_b) <= 19))
			continue;
		strcpy(rt_symbol->symb[ii].symb, symb_b);
		rt_symbol->symb[ii].mark = 1;
		ii++;
		if (ii >= no_of_symbols)
			break;
	}
	rt_symbol->valr = ii;
	qsort(rt_symbol->symb, rt_symbol->valr, sizeof(rt_symbol->symb[0]), _cmp_symb);
	for (ii = 0; ii < rt_symbol->valr; ii++)
	{
		rt_symbol->symb[ii].seqn = ii;
		strcpy(rt_board->hook[ii].symb, rt_symbol->symb[ii].symb);
		rt_board->hook[ii].mark = 1;
	}
	fclose(symbF);
	return(0);
}

/*
 * symbol_pre_load()
 * Pre load symbol file
 */
int symbol_pre_load()
{
	char	line_b[512], symb_b[128];
	FILE	*symbF;
	int	ii;

	if (strlen(preload_file) <= 0)
		return(0);
	if ((symbF = fopen(preload_file, "r")) == NULL)
		return(0);

	ii = 0;
	while ((fgets(line_b, sizeof(line_b), symbF)) == line_b)
	{
		symb_b[0] = '\0';
		sscanf(line_b, "%s", symb_b);
		if (!(strlen(symb_b) > 0 && strlen(symb_b) <= 19))
			continue;
		strcpy(rt_symbol->symb[ii].symb, symb_b);
		rt_symbol->symb[ii].mark = 1;
		ii++;
		if (ii >= no_of_symbols)
			break;
	}
	rt_symbol->valr = ii;
	qsort(rt_symbol->symb, rt_symbol->valr, sizeof(rt_symbol->symb[0]), _cmp_symb);
	for (ii = 0; ii < rt_symbol->valr; ii++)
	{
		rt_symbol->symb[ii].seqn = ii;
		strcpy(rt_board->hook[ii].symb, rt_symbol->symb[ii].symb);
		rt_board->hook[ii].mark = 1;
	}
	fclose(symbF);
	return(0);
}

static int _cmp_symb(struct symb *symb1, struct symb *symb2)
{
	return(strcmp(symb1->symb, symb2->symb));
}
