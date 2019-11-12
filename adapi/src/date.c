
#include "adapi_def.h"



int MYAPI DateGetMonthDays(int nMonth,int nYear)
{
	switch(nMonth)
	{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			return 31;
		case 4:
		case 6:
		case 9:
		case 11:
			return 30;
		case 2:
			if( (nYear%4==0 && nYear%100!=0) || nYear%400==0)
			return 29;
			else
			return 28;
		default:
			return 30;
	}
}
int MYAPI DateSubMonth(int *pYear,int *pMonth,int *pMday,int nSubMonth)
{
	int nBorrowYear=0;
	int nMonth,nDay;
	if(nSubMonth<0)
	{
		return DateAddMonth(pYear,pMonth,pMday,0-nSubMonth);

	}

	for(;(nMonth=(nBorrowYear*12+*pMonth)-nSubMonth)<=0;nBorrowYear++);
	*pMonth=nMonth;
	*pYear-=nBorrowYear;

	nDay=DateGetMonthDays(*pMonth,*pYear);
	if(*pMday>nDay)*pMday=nDay;
	return 0;

}
int MYAPI DateAddMonth(int *pYear,int *pMonth,int *pMday,int nAddMonth)
{
    int nDay;
//    int nMonth;
    if(nAddMonth<0)
    {
        return DateSubMonth(pYear,pMonth,pMday,0-nAddMonth);

    }

    while((*pMonth+nAddMonth)>12)
    {
        nAddMonth-=13-*pMonth;
        *pMonth=1;
        (*pYear)++;
    }
    *pMonth+=nAddMonth;

    nDay=DateGetMonthDays(*pMonth,*pYear);
    if(*pMday>nDay)*pMday=nDay;

    return 0;

}
int MYAPI DateAddDay(int *pYear,int *pMonth,int *pMday,int nAddDay)
{
	int nMonthDay;
	if(nAddDay<0)
	{
		return DateSubDay(pYear,pMonth,pMday,0-nAddDay);
	}

	while((*pMday+nAddDay)>(nMonthDay=DateGetMonthDays(*pMonth,*pYear)))
	{
		nAddDay-=nMonthDay-*pMday+1;
		*pMday=1;
		DateAddMonth(pYear,pMonth,pMday,1);
	}
	*pMday+=nAddDay;
	return 0;
}
int MYAPI DateSubDay(int *pYear,int *pMonth,int *pMday,int nSubDay)
{
	if(nSubDay<0)
	{
		return DateAddDay(pYear,pMonth,pMday,0-nSubDay);
	}

	while((*pMday-nSubDay)<=0)
	{
		nSubDay-=*pMday;
		DateSubMonth(pYear,pMonth,pMday,1);
		*pMday=DateGetMonthDays(*pMonth,*pYear);
	}
	*pMday-=nSubDay;
	return 0;
}
static char *_Months[12]=
{
	"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};
int MYAPI StrMonthToNumber(const char *month)
{
	int  i = 0 ;
	if( month == NULL || month[0] == 0 )
	{
		return -1;
	}
	for( i = 0 ; i < 12 ; i ++ )
	{
		if( strcmpipre( month ,_Months[i] ) == 0 )
		{
			return (i+1);
		}
	}
	return -1;
}
char * MYAPI MonthToStr(int nMonth,char *szMonth4Size)
{

    static char szUnkown[]="Unkown";
    if(nMonth>=1&&nMonth<=12)
    {
        nMonth--;
        if(szMonth4Size)
        {
            strcpyx(szMonth4Size,_Months[nMonth]);
            return szMonth4Size;
        }
        else
        {
            return _Months[nMonth];
        }
    }
    else
    {
        return szUnkown;
    }
}
static char *_Weeks[ 7 ] =
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
char * MYAPI WeekToStr(int nWeek,char *szWeek4Size)
{

    static char szUnkown[]="Unkown";
    if(nWeek>=0&&nWeek<=6)
    {
        if(szWeek4Size)
        {
            strcpyx( szWeek4Size, _Weeks[ nWeek ] );
            return szWeek4Size;
        }
        else
        {
            return _Weeks[ nWeek ];
        }
    }
    else
    {
        return szUnkown;
    }

}
int MYAPI StrWeekToNumber( const char *week )
{
    int  i = 0;
    if( week == NULL || week[ 0 ] == 0 )
    {
        return -1;
    }
    for( i = 0; i < 7; i++ )
    {
        if( strcmpipre( week, _Weeks[ i ] ) == 0 )
        {
            return ( i + 1 );
        }
    }
    return -1;
}

char * MYAPI time_t2Str(time_t tToday,char *out22len,BOOL bUTC,BOOL IsSplit)
{
    struct tm tmToday;
    if(!out22len)
    {
        return NULL;
    }
    if( bUTC )
    {
#ifdef WIN32
        tmToday =  *gmtime(&tToday);
#else
        gmtime_r(&tToday,&tmToday);
#endif
    }
    else
    {
#ifdef WIN32
        tmToday =  *localtime(&tToday);
#else
        localtime_r(&tToday,&tmToday);
#endif
    }

    tmToday.tm_year+=1900;
    tmToday.tm_mon+=1;
    if(IsSplit)
    {
        sprintf(out22len,"%04d-%02d-%02d %02d:%02d:%02d",tmToday.tm_year,tmToday.tm_mon,tmToday.tm_mday,
            tmToday.tm_hour,tmToday.tm_min,tmToday.tm_sec);

    }
    else
    {
        sprintf(out22len,"%04d%02d%02d%02d%02d%02d",tmToday.tm_year,tmToday.tm_mon,tmToday.tm_mday,
            tmToday.tm_hour,tmToday.tm_min,tmToday.tm_sec);
    }
    return out22len;
}
SYSTEMTIME * MYAPI HttpDate2Systime( const char *szDate, SYSTEMTIME *systime )
{
    char szMonth[ 8 ];
    int  i = 0 ;
    char szTmp[ 40 ];
    char *pszTmp = NULL;
    strcpyn( szTmp, szDate, sizeof( szTmp ) );

    memset( systime, 0, sizeof( SYSTEMTIME ) );
    
    for( ; szTmp[i] == ' ' ; i ++ );
    pszTmp = szTmp + i;

    for( ; szTmp[ i ] != 0 && szTmp[ i ] != ','; i++ );

    if( szTmp[ i ] != ',' )
    {
        return  systime ;
    }
    szTmp[ i ] = 0 ;
    systime->wDayOfWeek = StrWeekToNumber( pszTmp );

    i ++ ;
    for( ; szTmp[ i ] == ' '; i++ );
    pszTmp = szTmp + i;
    for( ; szTmp[ i ] != 0 && szTmp[ i ] != ' '; i++ );
    if( szTmp[ i ] != ' ' )
    {
        return  systime;
    }
    szTmp[ i ] = 0;
    systime->wDay = atoi( pszTmp );

    i++;
    for( ; szTmp[ i ] == ' '; i++ );
    pszTmp = szTmp + i;
    for( ; szTmp[ i ] != 0 && szTmp[ i ] != ' '; i++ );
    if( szTmp[ i ] != ' ' )
    {
        return  systime;
    }
    szTmp[ i ] = 0;
    systime->wMonth = StrMonthToNumber( pszTmp );

    i++;
    for( ; szTmp[ i ] == ' '; i++ );
    pszTmp = szTmp + i;
    for( ; szTmp[ i ] != 0 && szTmp[ i ] != ' '; i++ );
    if( szTmp[ i ] != ' ' )
    {
        return  systime;
    }
    szTmp[ i ] = 0;
    systime->wYear = atoi( pszTmp );

    i++;
    for( ; szTmp[ i ] == ' '; i++ );
    pszTmp = szTmp + i;
    for( ; szTmp[ i ] != 0 && szTmp[ i ] != ':'; i++ );
    if( szTmp[ i ] != ':' )
    {
        return  systime;
    }
    szTmp[ i ] = 0;
    systime->wHour = atoi( pszTmp );

    i++ ;
    pszTmp = szTmp + i;
    for( ; szTmp[ i ] != 0 && szTmp[ i ] != ':'; i++ );
    if( szTmp[ i ] != ':' )
    {
        return  systime;
    }
    szTmp[ i ] = 0;
    systime->wMinute = atoi( pszTmp );


    i++;
    pszTmp = szTmp + i;
    for( ; szTmp[ i ] != 0 && szTmp[ i ] != ' '; i++ );
    szTmp[ i ] = 0;
    systime->wSecond = atoi( pszTmp );

    return systime ;
}
char * MYAPI Systime2HttpDate( const SYSTEMTIME *systime, char *out30len )
{
    char szWeek[ 8 ];
    char szMonth[ 8 ];

    WeekToStr( systime->wDayOfWeek, szWeek  );
    MonthToStr( systime->wMonth , szMonth );

    sprintf( out30len, "%s, %02d %s %04d %02d:%02d:%02d GMT", szWeek, systime->wDay, szMonth, systime->wYear ,
             systime->wHour, systime->wMinute, systime->wSecond );

    return out30len;
}
char * MYAPI Systime2HttpCookieDate(const SYSTEMTIME *systime, char *out30len)
{
	char szWeek[8];
	char szMonth[8];

	WeekToStr(systime->wDayOfWeek, szWeek);
	MonthToStr(systime->wMonth, szMonth);

	sprintf(out30len, "%s, %02d-%s-%04d %02d:%02d:%02d GMT", szWeek, systime->wDay, szMonth, systime->wYear,
		systime->wHour, systime->wMinute, systime->wSecond);

	return out30len;
}
char * MYAPI Systime2Str(const SYSTEMTIME *systime, char *out22len,BOOL IsSplit)
{
    if(IsSplit)
    {
        sprintf(out22len,"%04d-%02d-%02d %02d:%02d:%02d",
                systime->wYear,systime->wMonth,systime->wDay,
                systime->wHour,systime->wMinute,systime->wSecond);
    }
    else
    {
        sprintf(out22len,"%04d%02d%02d%02d%02d%02d",
                systime->wYear,systime->wMonth,systime->wDay,
                systime->wHour,systime->wMinute,systime->wSecond);
    }
    
    return out22len ;
}
char * MYAPI DateTimeNowStr(char *out22len,BOOL bUTC,BOOL IsSplit)
{

#ifdef WIN32
    SYSTEMTIME systime;
    if( bUTC )
    {
        GetSystemTime(&systime);
    }
    else
    {
        GetLocalTime(&systime);
    }

	if(IsSplit)
	{
        sprintf(out22len,"%04d-%02d-%02d %02d:%02d:%02d",
            systime.wYear,systime.wMonth,systime.wDay,
            systime.wHour,systime.wMinute,systime.wSecond);
	}
	else
	{
        sprintf(out22len,"%04d%02d%02d%02d%02d%02d",
            systime.wYear,systime.wMonth,systime.wDay,
            systime.wHour,systime.wMinute,systime.wSecond);
	}
#else
    struct tm tmToday;
    time_t tToday=0;
    if(!out22len)return 0;
    tToday=time(0);
    if( bUTC )
    {
        gmtime_r(&tToday,&tmToday);
    }
    else
    {
        localtime_r(&tToday,&tmToday);
    }
    tmToday.tm_year+=1900;
    tmToday.tm_mon+=1;
	if(IsSplit)
	{
        sprintf(out22len,"%04d-%02d-%02d %02d:%02d:%02d",tmToday.tm_year,tmToday.tm_mon,tmToday.tm_mday,
            tmToday.tm_hour,tmToday.tm_min,tmToday.tm_sec);

	}
	else
	{
        sprintf(out22len,"%04d%02d%02d%02d%02d%02d",tmToday.tm_year,tmToday.tm_mon,tmToday.tm_mday,
            tmToday.tm_hour,tmToday.tm_min,tmToday.tm_sec);
	}
#endif
    return out22len;

}
SYSTEMTIME * MYAPI time_t2SystemTime(time_t tm1 , SYSTEMTIME *pSysTime)
{
#ifdef WIN32
	struct tm  *ptmTmp = 0;
	ptmTmp = gmtime(&tm1);
	return tm2SystemTime(ptmTmp, pSysTime);
#else
	struct tm  tmTmp;
	gmtime_r(&tm1, &tmTmp);
	return tm2SystemTime(&tmTmp, pSysTime);;
#endif
}
SYSTEMTIME * MYAPI tm2SystemTime(const struct tm *pTm,SYSTEMTIME *pSysTime)
{
	if( !pTm || !pSysTime )
	{
		return pSysTime;
	}
	pSysTime->wYear = pTm->tm_year + 1900;
	pSysTime->wMonth = pTm->tm_mon  + 1;
	pSysTime->wDay = pTm->tm_mday ;
	pSysTime->wHour = pTm->tm_hour ;
	pSysTime->wMinute = pTm->tm_min ;
	pSysTime->wSecond = pTm->tm_sec ;
	pSysTime->wMilliseconds = 0;
	pSysTime->wDayOfWeek =pTm->tm_wday ;
	return pSysTime;
}

struct tm * MYAPI SystemTime2tm(const SYSTEMTIME *pSysTime,struct tm *pTm)
{
	if( !pTm || !pSysTime )
	{
		return pTm;
	}
	memset(pTm,0,sizeof(struct tm));
	pTm->tm_year = pSysTime->wYear - 1900 ;
	pTm->tm_mon = pSysTime->wMonth -1 ;
	pTm->tm_mday = pSysTime->wDay ;
	pTm->tm_hour = pSysTime->wHour ;
	pTm->tm_min = pSysTime->wMinute ;
	pTm->tm_sec = pSysTime->wSecond  ;
	pTm->tm_wday = pSysTime->wDayOfWeek ;
	mktime(pTm);

	return pTm;
}
SYSTEMTIME * MYAPI SystemTimeAddWeek(SYSTEMTIME *pSysTime)
{
    struct tm tmTmp ;
    if( !pSysTime )
    {
        return NULL ;
    }
	SystemTime2tm(pSysTime,&tmTmp);
    pSysTime->wDayOfWeek = tmTmp.tm_wday ;
    return pSysTime;
}

time_t MYAPI time_t2Local(time_t tSysTime)
{
#ifdef WIN32
    struct tm  *ptmTmp = 0;
    time_t ttLocal = 0;
    ttLocal = time(0);
    ptmTmp = gmtime(&ttLocal);
    tSysTime += (ttLocal - mktime(ptmTmp)) ;

    ptmTmp = localtime(&tSysTime);
    if( !ptmTmp )
    {
        return tSysTime;
    }
    return mktime(ptmTmp);
#else
    struct tm  tmTmp ;
    time_t ttLocal = 0;
    ttLocal = time(0);
    gmtime_r(&ttLocal,&tmTmp);
    tSysTime += (ttLocal - mktime(&tmTmp)) ;


    localtime_r(&tSysTime,&tmTmp);

    return mktime(&tmTmp);
#endif
}
const char * MYAPI Seconds2TimeStr(int nSecondsIn,char *szBuf,int nLen,BOOL bSplit)
{
    char szTimeStr[80];
    int nHour = 0;
    int nMinutes = 0;
    int nSecondes = 0;
    static char szNeg[] = "-";
    static char szNULL[] = "\0";
    char *pszNeg;

    if( ! szBuf )
    {
        return NULL ;
    }
    szBuf[0] = 0 ;
    if( nSecondsIn < 0 )
    {
        nSecondsIn = 0 - nSecondsIn ;
        pszNeg = szNeg ;
    }
    else
    {
        pszNeg = szNULL ;
    }


    memset( szTimeStr, 0, _SIZE_OF(szTimeStr) );

    nHour = nSecondsIn / 3600 ;

    nMinutes = (nSecondsIn % 3600) / 60 ;

    nSecondes =  (nSecondsIn % 3600) % 60 ;

    if( ! bSplit && nHour > 99 )
    {
        nHour = 99 ;
    }
    if( bSplit )
    {
        snprintf(szTimeStr,_SIZE_OF(szTimeStr)-1,
            "%s%02d:%02d:%02d",pszNeg,nHour,nMinutes,nSecondes );
    }
    else
    {
        snprintf(szTimeStr,_SIZE_OF(szTimeStr)-1,
            "%s%02d%02d%02d",pszNeg,nHour,nMinutes,nSecondes );
    }
    strcpyn(szBuf,szTimeStr,nLen);
    return szBuf ;
}
int MYAPI TimeStr2Seconds(const char *szTimeIn)
{
    int nHour = 0;
    int nMinutes = 0;
    int nSecondes = 0;
    int nSecondesRet = 0;
    int i=0;
    char szTmp[8];
    const char *pszTmp = 0;
    char szTime[40];
    BOOL bPM = FALSE ;
    BOOL bNeg = FALSE ;

    if( !szTimeIn || !szTimeIn[0] )
    {
        return -1;
    }
    memset(szTime,0,_SIZE_OF(szTime));

    if( szTimeIn[0] == '-' )
    {
        szTimeIn ++ ;
        bNeg = TRUE ;
    }
    strcpyn(szTime,szTimeIn,40);
    pszTmp = szTime;

    i = strcmpifind(szTime,":");
    if( strcmpifind(szTime,"PM") > 1 )
    {
        bPM = TRUE ;
    }

    if( i>0 )
    {
        strcpyn_stop(szTmp,pszTmp,8,":");
        nHour = atoix(szTmp);
        i = strcmpifind(pszTmp,":");
        if(i<1)
        {
            goto LABLE_FAIL;
        }
        pszTmp = pszTmp+i+1;
        strcpyn_stop(szTmp,pszTmp,8,":");
        nMinutes = atoix(szTmp);

        i = strcmpifind(pszTmp,":");
        if(i<1)
        {
            goto LABEL_OK;
        }
        pszTmp = pszTmp+i+1;
        strcpyn_stop(szTmp,pszTmp,8,":");
        nSecondes = atoix(szTmp);
    }
    else if( i<0 )
    {

        memcpy(szTmp,szTime,2);
        szTmp[2]=0;
        nHour = atoix(szTmp);

        memcpy(szTmp,szTime+2,2);
        szTmp[2]=0;
        nMinutes = atoix(szTmp);

        memcpy(szTmp,szTime+2+2,2);
        szTmp[2]=0;
        nSecondes = atoix(szTmp);
    }
    else
    {
        return -1;
    }

LABEL_OK: ;
    if( nHour > 23 || nHour < 0 )
    {
        goto LABLE_FAIL;
    }
    if( nMinutes > 59 || nMinutes < 0 )
    {
        goto LABLE_FAIL;
    }
    if( nSecondes > 59 || nSecondes < 0 )
    {
        goto LABLE_FAIL;
    }

    if( bPM && nHour < 12 )
    {
        nSecondesRet = ( (nHour*3600)+(nMinutes*60)+nSecondes)+12*3600;

    }
    else
    {
        nSecondesRet = ((nHour*3600)+(nMinutes*60)+nSecondes);

    }

    if( bNeg )
    {
        return ( 0 - nSecondesRet );
    }
    else
    {
        return nSecondesRet ;
    }

LABLE_FAIL:
    return -1;
}
SYSTEMTIME * MYAPI DateTimeStr2SystemTime(const char *szTimeIn, SYSTEMTIME *pSysTime,BOOL *pbSuccess)
{
    int nYear = 0;
    int nMonth = 0;
    int nDay = 0;
    int nHour = 0;
    int nMinutes = 0;
    int nSecondes = 0;
    char szTmp[8];
    char szTime[100];
    const char *pszTmp = 0;
    int i=0;
    if( !szTimeIn || !szTimeIn[0] )
    {
        if( pbSuccess )
        {
            *pbSuccess = FALSE ;
        }
        return pSysTime;
    }
    strcpyn(szTime,szTimeIn,100);
    i = strcmpifind(szTime,"-UTC");
    if( i>0 )
    {
        szTime[i] = 0 ;
    }
    i = strcmpifind(szTime,"-");
    if( i >0 )
    {
        strcpyn_stop(szTmp,szTime,8,"-");
        nYear = atoix(szTmp);
        pszTmp = szTime+i+1;
        strcpyn_stop(szTmp,pszTmp,8,"-");
        nMonth = atoix(szTmp);
        i = strcmpifind(pszTmp,"-");
        if(i<1)
        {
            goto LABLE_FAIL;
        }
        pszTmp = pszTmp+i+1;
        strcpyn_stop(szTmp,pszTmp,8," T\t");
        nDay = atoix(szTmp);

        for (; *pszTmp && (*pszTmp) != ' ' && (*pszTmp) != 'T' && (*pszTmp) != '\t'; pszTmp++);
        for (; *pszTmp && ((*pszTmp) == ' ' || (*pszTmp) == 'T' || (*pszTmp) == '\t'); pszTmp++);

        strcpyn_stop(szTmp,pszTmp,8,":");
        nHour = atoix(szTmp);
        i = strcmpifind(pszTmp,":");
        if(i<1)
        {
            goto LABLE_FAIL;
        }
        pszTmp = pszTmp+i+1;
        strcpyn_stop(szTmp,pszTmp,8,":");
        nMinutes = atoix(szTmp);

        i = strcmpifind(pszTmp,":");
        if(i > 0 )
        {
			pszTmp = pszTmp+i+1;
			strcpyn_stop(szTmp,pszTmp,8,":");
			nSecondes = atoix(szTmp);
        }

    }
    else
    {
        if( strlenx(szTime) < 14 )
        {
            goto LABLE_FAIL;
        }

        memcpy(szTmp,szTime,4);
        szTmp[4]=0;
        nYear = atoix(szTmp);

        memcpy(szTmp,szTime+4,2);
        szTmp[2]=0;
        nMonth = atoix(szTmp);

        memcpy(szTmp,szTime+4+2,2);
        szTmp[2]=0;
        nDay = atoix(szTmp);

        memcpy(szTmp,szTime+4+2+2,2);
        szTmp[2]=0;
        nHour = atoix(szTmp);

        memcpy(szTmp,szTime+4+2+2+2,2);
        szTmp[2]=0;
        nMinutes = atoix(szTmp);

        memcpy(szTmp,szTime+4+2+2+2+2,2);
        szTmp[2]=0;
        nSecondes = atoix(szTmp);
    }
    if( nMonth > 12 || nMonth < 1 )
    {
        goto LABLE_FAIL;
    }
    if( nDay > 31 || nDay < 1 )
    {
        goto LABLE_FAIL;
    }
    if( nHour > 23 || nHour < 0 )
    {
        goto LABLE_FAIL;
    }
    if( nMinutes > 59 || nMinutes < 0 )
    {
        goto LABLE_FAIL;
    }
    if( nSecondes > 59 || nSecondes < 0 )
    {
    //    goto LABLE_FAIL;
    }

    if( pSysTime )
    {
        memset(pSysTime,0,_SIZE_OF(SYSTEMTIME));
        pSysTime->wYear = (WORD)nYear;
        pSysTime->wMonth = (WORD)nMonth;
        pSysTime->wDay =(WORD)nDay;

        pSysTime->wHour = (WORD)nHour;
        pSysTime->wMinute = (WORD)nMinutes;
        pSysTime->wSecond = (WORD)nSecondes;
    }
    if( pbSuccess )
    {
        *pbSuccess = TRUE ;
    }
    return pSysTime;
LABLE_FAIL:
    if( pSysTime )
    {
        memset(pSysTime,0,_SIZE_OF(SYSTEMTIME));
        if( pbSuccess )
        {
            *pbSuccess = FALSE ;
        }
    }
    return pSysTime;
}

SYSTEMTIME *MYAPI SystemTime( SYSTEMTIME *pSysTime, BOOL bUTC )
{


#if defined(_WIN32) || defined(WIN32)
	if( !pSysTime )
	{
		return 0;
	}
    if( bUTC )
    {
        GetSystemTime(pSysTime);
    }
    else
    {
        GetLocalTime(pSysTime);
    }

#else
	time_t timeSys;
#if !defined(_NO_TIME_R_)
    struct tm tmTmp;
#endif
	if( !pSysTime )
	{
		return 0;
	}
	timeSys = time(0);
#if !defined(_NO_TIME_R_)
    if( bUTC )
    {
        gmtime_r(&timeSys,&tmTmp);
    }
    else
    {
        localtime_r(&timeSys,&tmTmp);
    }
    tm2SystemTime(&tmTmp,pSysTime);
#else
    if( bUTC )
    {
        tm2SystemTime(gmtime(&timeSys),pSysTime);
    }
    else
    {
        tm2SystemTime(localtime(&timeSys),pSysTime);
    }
#endif

#endif
	return pSysTime;
}

time_t MYAPI SystemTime2time_t(const SYSTEMTIME *pSysTime)
{
	struct tm tmTime;
	SystemTime2tm(pSysTime,&tmTime);
	return (time_t)mktime(&tmTime);
}


TIME_T MYAPI OSUptime()
{
#if defined(_WIN32) || defined(WIN32)
    return (TIME_T)( GetTickCount() / 1000.0 );
#else
    static time_t nTimeFirst = 1;
#ifdef _FAST_OSUPTIME_
    if(  nTimeFirst == 1 )
    {
        nTimeFirst = time(0) ;
        return 1;
    }
    return (time(0)-nTimeFirst) ;
#else
    static BOOL bOsUpTmSupported = TRUE;


#ifdef _OS_UPTIME_RIGOR_
    static TIME_T tupTimePre=0;
#endif

    TIME_T nTime;
    char szUptime[100];
    FILE_HANDLE  hFileUptime = FILE_HANDLE_INVALID ;
    int nRet = 0, i = 0 ;

LABLE_NOTSUPPORT_UPTIME:
    if( !bOsUpTmSupported )
    {
        if( nTimeFirst == 0 )
        {
            nTimeFirst = time(0);
            return 0;
        }
        else
        {
#ifdef _OS_UPTIME_RIGOR_
            return ( (TIME_T)(time(0)-nTimeFirst)+ tupTimePre );
#else
            return ( (TIME_T)(time(0)-nTimeFirst) );
#endif
        }

    }

        hFileUptime = FileOpenXX("/proc/uptime",OX_RDONLY);
        if( hFileUptime == FILE_HANDLE_INVALID )
        {
            bOsUpTmSupported = FALSE;
            goto LABLE_NOTSUPPORT_UPTIME;
            return 0;
        }


	nRet = FileReadXX(hFileUptime,szUptime,99);
	if( nRet< 1 )
	{
		FileCloseXX(hFileUptime);
        bOsUpTmSupported = FALSE;
        goto LABLE_NOTSUPPORT_UPTIME;
		return 0;
	}
	szUptime[nRet] = 0;
	for( i =0 ; szUptime[i] ; i++ )
	{
		if( szUptime[i] == '.' || szUptime[i] ==' ' )
		{
			szUptime[i]  = 0;
			break;
		}
	}
	FileCloseXX(hFileUptime);

#if defined __USE_ISOC99 || (defined __GLIBC_HAVE_LONG_LONG && defined __USE_MISC)
	nTime = (TIME_T)strtoll(szUptime,0,10);
#else
    nTime = (TIME_T)atoi(szUptime);
#endif

#ifdef _OS_UPTIME_RIGOR_
    tupTimePre = nTime;
    nTimeFirst = time(0);
#endif
    return nTime;
#endif

#endif
}
TIME_T MYAPI OSTickCount()
{
#if defined(_WIN32) || defined(WIN32)
    return (TIME_T)( GetTickCount() );
#elif defined(_IOS_) || defined(_Darwin_)
	static uint64_t _tickStart = 0 ;
    TIME_T millisecs = 0 ;
	static mach_timebase_info_data_t _tickInfo;
	uint64_t nanosecs = 0 ;
	uint64_t elapsed = 0 ;
	if ( _tickStart == 0 )
	{
		_tickStart = mach_absolute_time() ;
		if (mach_timebase_info (&_tickInfo) != KERN_SUCCESS) 
		{
			printf ("mach_timebase_info failed\n");
		}
	}
	elapsed =  mach_absolute_time() - _tickStart ;

	nanosecs = elapsed * _tickInfo.numer / _tickInfo.denom;
	millisecs = nanosecs / 1000000;

	return millisecs;
#elif defined( CLOCK_MONOTONIC_RAW )
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#elif defined( CLOCK_MONOTONIC )
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#else
    static TIME_T tupTimeBoot=0;
    TIME_T tupTime = 0;
    struct timeval tv = {0,0};
    if( tupTimeBoot == 0 )
    {
        tupTime = 0;
        gettimeofday(&tv,0);
        tupTimeBoot = tv.tv_sec - tupTime ;
        return (tupTime*1000 + (tv.tv_usec/1000));
    }
    else
    {
        gettimeofday(&tv,0);
        tupTime = tv.tv_sec - tupTimeBoot;
		if( tupTime < 0 )
		{
			tupTime = 0 ;
			tupTimeBoot = tv.tv_sec ;
		}
        return (tupTime*1000 + (tv.tv_usec/1000));
    }
#endif
}

TIME_T MYAPI OSUptimeMilliEx()
{
#if defined(_WIN32) || defined(WIN32)
    return (TIME_T)( GetTickCount() );
#else

    static BOOL bOsUpTmSupported = TRUE;

    TIME_T nTime;
    char szUptime[100];
    FILE_HANDLE  hFileUptime = FILE_HANDLE_INVALID ;
    int nRet = 0, i = 0 ;

LABLE_NOTSUPPORT_UPTIME:
    if( !bOsUpTmSupported )
    {
        return OSTickCount();
    }

    hFileUptime = FileOpenXX("/proc/uptime",OX_RDONLY);
    if( hFileUptime == FILE_HANDLE_INVALID )
    {
        bOsUpTmSupported = FALSE;
        goto LABLE_NOTSUPPORT_UPTIME;
        return 0;
    }


    nRet = FileReadXX(hFileUptime,szUptime,99);
    if( nRet< 1 )
    {
        FileCloseXX(hFileUptime);
        bOsUpTmSupported = FALSE;
        goto LABLE_NOTSUPPORT_UPTIME;
        return 0;
    }
    szUptime[nRet] = 0;
    for( i =0 ; szUptime[i] ; i++ )
    {
        if( szUptime[i] == ' ' )
        {
            szUptime[i]  = 0;
            break;
        }
    }
    FileCloseXX(hFileUptime);

#if defined __USE_ISOC99 || (defined __GLIBC_HAVE_LONG_LONG && defined __USE_MISC)
    nTime = (TIME_T)(strtod(szUptime,0)*1000.0);
#else
    nTime = (TIME_T)(atof(szUptime)*1000.0);
#endif

    return nTime;
#endif

}

TIME_T MYAPI OSUptimeMilli()
{
#if defined(_WIN32) || defined(WIN32)
    return (TIME_T)( GetTickCount() );
#else

#ifdef _FAST_OSUPTIME_
    return OSTickCount();
#else
    static BOOL bOsUpTmSupported = TRUE;

    TIME_T nTime;
    char szUptime[100];
    FILE_HANDLE  hFileUptime = FILE_HANDLE_INVALID ;
    int nRet = 0, i = 0 ;

LABLE_NOTSUPPORT_UPTIME:
    if( !bOsUpTmSupported )
    {
        return OSTickCount();
    }

    hFileUptime = FileOpenXX("/proc/uptime",OX_RDONLY);
    if( hFileUptime == FILE_HANDLE_INVALID )
    {
        bOsUpTmSupported = FALSE;
        goto LABLE_NOTSUPPORT_UPTIME;
        return 0;
    }


    nRet = FileReadXX(hFileUptime,szUptime,99);
    if( nRet< 1 )
    {
        FileCloseXX(hFileUptime);
        bOsUpTmSupported = FALSE;
        goto LABLE_NOTSUPPORT_UPTIME;
        return 0;
    }
    szUptime[nRet] = 0;
    for( i =0 ; szUptime[i] ; i++ )
    {
        if( szUptime[i] == ' ' )
        {
            szUptime[i]  = 0;
            break;
        }
    }
    FileCloseXX(hFileUptime);

#if defined __USE_ISOC99 || (defined __GLIBC_HAVE_LONG_LONG && defined __USE_MISC)
    nTime = (TIME_T)(strtod(szUptime,0)*1000.0);
#else
    nTime = (TIME_T)(atof(szUptime)*1000.0);
#endif

    return nTime;
#endif
#endif
}
#define _SYSTIME_COMPARE_SIMPLE_
int MYAPI SystemTimeCompare(const SYSTEMTIME *pSysTime,const SYSTEMTIME *pSysTime2)
{
#ifdef _SYSTIME_COMPARE_SIMPLE_
    int nRet = 0;
    if( !pSysTime || !pSysTime2 )
    {
        return -1;
    }

    nRet = pSysTime->wYear - pSysTime2->wYear ;
    if( nRet > 0 )
    {
        return 1 ;
    }
    else if( nRet < 0 )
    {
        return -1 ;
    }

    nRet = pSysTime->wMonth - pSysTime2->wMonth ;
    if( nRet > 0 )
    {
        return 1 ;
    }
    else if( nRet < 0 )
    {
        return -1 ;
    }

    nRet = pSysTime->wDay - pSysTime2->wDay ;
    if( nRet > 0 )
    {
        return 1 ;
    }
    else if( nRet < 0 )
    {
        return -1 ;
    }

    nRet = pSysTime->wHour - pSysTime2->wHour ;
    if( nRet > 0 )
    {
        return 1 ;
    }
    else if( nRet < 0 )
    {
        return -1 ;
    }

    nRet = pSysTime->wMinute - pSysTime2->wMinute ;
    if( nRet > 0 )
    {
        return 1 ;
    }
    else if( nRet < 0 )
    {
        return -1 ;
    }

    nRet = pSysTime->wSecond - pSysTime2->wSecond ;
    if( nRet > 0 )
    {
        return 1 ;
    }
    else if( nRet < 0 )
    {
        return -1 ;
    }

    nRet = pSysTime->wMilliseconds - pSysTime2->wMilliseconds ;
    if( nRet > 0 )
    {
        return 1 ;
    }
    else if( nRet < 0 )
    {
        return -1 ;
    }

    return 0;
#else
    return (int)(SystemTime2time_t(pSysTime)- SystemTime2time_t(pSysTime2)) ;
#endif
}

int MYAPI SecondsFromMidNight()
{
    SYSTEMTIME sysTm;
    SystemTime( &sysTm, FALSE );

    return sysTm.wHour * 3600 + sysTm.wMinute * 60 + sysTm.wSecond;
}
BOOL MYAPI SystemTimeSet( const SYSTEMTIME *pSysTime, BOOL bUTC )
{
#ifdef WIN32
    if( !pSysTime || !pSysTime->wYear )
    {
        return FALSE;
    }
    if( bUTC )
    {

        return SetSystemTime(pSysTime);
    }
    else
    {
        return SetLocalTime(pSysTime);
    }
#else
    struct timeval tmSys={0,0};
    int nRet = 0;
    if( !pSysTime || !pSysTime->wYear )
    {
        return FALSE;
    }

    tmSys.tv_sec = SystemTime2time_t(pSysTime);
    tmSys.tv_usec = pSysTime->wMilliseconds * 1000 ;
    if( bUTC )
    {
        tmSys.tv_sec = time_t2Local(tmSys.tv_sec);

        nRet = settimeofday(&tmSys,0);
    }
    else
    {
        nRet = settimeofday(&tmSys,0);
    }
    return (nRet==0);
#endif

}

int MYAPI WeekDay()
{
    SYSTEMTIME sysTM;

    SystemTime(&sysTM,FALSE);
    if( (short int)sysTM.wDayOfWeek < 0 )
    {
        sysTM.wDayOfWeek = 0 ;
    }
    if( sysTM.wDayOfWeek > 6 )
    {
        sysTM.wDayOfWeek = 6 ;
    }
    return sysTM.wDayOfWeek;
}





