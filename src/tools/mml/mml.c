/*
 *Convert MML to TrackData for PC-Engine
 * Ver 0.01 05/Oct./1995
 *
 *(c)Orc
 */

#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include<math.h>

#define OBJ_STR_SIZE 1000
#define ON	1
#define OFF	0

char *short_help="MML: Convert MML to TrackData for PC-Engine.\n";
char buf[1024];
char buf2[100];

int instr( char* base, char obj );
char *remove_ext(char* string);
char *fgetstr(char*,FILE*,char*);

int main(int argc,char *argv[])
{
	char infilename[13];
	char a,channel=0,pan_r,pan_l,ext=OFF;
	char f_channel=OFF;/* トラックデータ先アドレス登録テーブルがあるか */
	char pc_mode=0;/* パーカッションモード */
	static char ch[7]={0,OFF,OFF,OFF,OFF,OFF,OFF};
	static int ch_length[7]={4,4,4,4,4,4,4};
	static int amari[7]={0,0,0,0,0,0,0};
	int i,j,k,line=0,length=0,put_length,tone;
	double futen;
	FILE *infile,*outfile;
	
	
	if( argc != 2 ){
		puts(short_help);
		puts("\tUsagi:MML [mml-file]\n");
		exit(1);
	}
	
	for(i=0; argv[1][i]!=0 ;i++){
		if(argv[1][i]=='.'){
			ext=ON;
			break;
		}
	}
	
	strcpy(infilename,argv[1]);
	if(ext==OFF){
		strcat(infilename,".MML");
	}
	if( (infile=fopen(infilename,"r")) == NULL ){
		printf("File not found:[%s]\n",infilename);
		exit(1);
	}
	printf("INFILE:%s\n",infilename);
	
	strcpy(buf,remove_ext(infilename));
	strcat(buf,".asm");
	if( (outfile=fopen(buf,"w")) == NULL ){
		printf("\nCan't open OUTPUT_FILE.\n");
		exit(1);
	}
	
	/* 出力開始 */
	fputs("\torg\t\t$8000\n", outfile);
	
	/* トラックデータインデックステーブル */
	fputs("\
track_index:\n\
\tdw\t\ttrack0\n\
;\n", outfile);
	/* ループ開始 */
	while((i=fgetc(infile)) != EOF){
		line++;
		switch((char)i){
			case '.':
			/* 指令である */
				fgetstr(buf,infile,"=");
				if( /*strnicmp*/strncasecmp(buf, "START", 5) == 0 ){
					channel=buf[5]-'0';
					if( channel<1 || channel>6 ){
						printf("Channel Error in %s:%d\n",infilename,line);
						exit(1);
					}
					if( ch[channel] == ON ){
						printf("Channel [%d] already exist in %s:%d",channel,infilename,line);
						exit(1);
					}
					
					ch[channel]=ON;
					f_channel=ON;
					ch_length[channel]=4;
					pc_mode=0;
					fputs("START_CH", outfile);
					fputc(buf[5], outfile);
					fputs(":\n" ,outfile);
					goto MML_EXT;
				}
				break;
			case ' ':
			case '\t':
			/* 前の続き */
				goto MML_EXT;
			case ';':
			/* コメントである */
				fgetstr(buf, infile, "\n\r");
			case '\n':
			case '\r':
				break;
			default:
			/* それ以外は文字列 */
				ungetc((char)i, infile);
				channel=0;
				ch_length[channel]=4;
				pc_mode=0;
				fgetstr(buf2, infile, "=");
				strcpy(buf,"LABEL_");
				strcat(buf,buf2);
				strcat(buf,":\n");
				fputs( buf, outfile);
MML_EXT:
				j=0;
				fgetstr(buf,infile,";\n\r");
				/* MML展開ループ */
				while(buf[j] != 0){
					
					/* 通常の音符 */
					if( (k=instr("C D EF G A B", buf[j])) != 0 && pc_mode==0 ){
						j++;
						if(buf[j]=='#' || buf[j]=='+'){
							k++; /* 半音上げる */
							j++;
						}else if(buf[j]=='-'){
							k--; /* 半音下げる */
							j++;
						}
						/* 音長 */
						length=0;
						while( isdigit((int)buf[j]) ){
							length = length*10+buf[j]-'0';
							j++;
						}
						switch(length){
							case 0:
								length=ch_length[channel];
								break;
							case 1:
							case 2:
							case 3:
							case 4:
							case 6:
							case 8:
							case 12:
							case 16:
							case 24:
							case 32:
							case 48:
							case 64:
							case 96:
								break;
							default:
								printf("Length error in:%d(%d)\n",line,length);
								exit(1);
						}
						/* kが1〜12以外のときの前処理 */
						if(k==0){
							fputs("\tdb\t\t$d9    \t;ｵｸﾀｰﾌﾞﾀﾞｳﾝ\n", outfile);
						}else if(k==13){
							fputs("\tdb\t\t$d8    \t;ｵｸﾀｰﾌﾞｱｯﾌﾟ\n", outfile);
						}
						tone=k;
						if(k==0)tone=12;
						if(k==13)tone=1;
						tone*=16;
						/* ダイレクトレングスモード */
						put_length=(int)( (192+amari[channel])/length);
						amari[channel]=(int)( (192+amari[channel])%length);
						/* 符点処理 */
						futen=1.0;
						length=1;
						while(buf[j]=='.'){
							futen += pow(0.5, length++);
							j++;
						}
						put_length*=futen;
						fprintf(outfile, "\tdb\t\t$%02x,$%02x\t;ｵﾝﾌﾟ\n", tone, put_length);
					
						/* kが1〜12以外のときの後処理 */
						if(k==0){
							fputs("\tdb\t\t$d8    \t;ｵｸﾀｰﾌﾞｱｯﾌﾟ\n", outfile);
						}else if(k==13){
							fputs("\tdb\t\t$d9    \t;ｵｸﾀｰﾌﾞﾀﾞｳﾝ\n", outfile);
						}	/* 音長終わり */
					
					/* 休符 */
					}else if( buf[j]=='R' && pc_mode==0 ){
						j++;
						/* 休符長 */
						length=0;
						while( isdigit((int)buf[j]) ){
							length = length*10+buf[j]-'0';
							j++;
						}
						switch(length){
							case 0:
								length=4;
								break;
							case 1:
							case 2:
							case 3:
							case 4:
							case 6:
							case 8:
							case 12:
							case 16:
							case 24:
							case 32:
							case 48:
							case 64:
							case 96:
								break;
							default:
								printf("Length error in:%d(%d)\n",line,length);
								exit(1);
						}
						put_length=(192/length);
						/* 符点処理 */
						futen=1.0;
						length=1;
						while(buf[j]=='.'){
							futen += pow(0.5, length++);
							j++;
						}
						put_length*=futen;
						fprintf(outfile, "\tdb\t\t$00,$%02x\t;ｷｭｳﾌ\n",put_length);
					
					/* オクターブ */
					}else if(buf[j] == 'O'){
						j++;
						/* オクターブ高 */
						length=0;
						while( isdigit((int)buf[j]) ){
							length = length*10+buf[j]-'0';
							j++;
						}
						if(length<1 || 7<length){
							printf("Parameter error of 'O' in:%d (%d)\n",line,length);
							exit(1);
						}
						fprintf(outfile, "\tdb\t\t$%02x    \t;ｵｸﾀｰﾌﾞ\n",0xD0+length);
					
					/* オクターブup */
					}else if(buf[j] == '>'){
						j++;
						fputs("\tdb\t\t$d8    \t;ｵｸﾀｰﾌﾞｱｯﾌﾟ\n", outfile);
					
					/* オクターブdown */
					}else if(buf[j] == '<'){
						j++;
						fputs("\tdb\t\t$d9    \t;ｵｸﾀｰﾌﾞﾀﾞｳﾝ\n", outfile);
					
					/* タイ */
					}else if(buf[j] == '&'){
						j++;
						fputs("\tdb\t\t$da   \t;ﾀｲ\n", outfile);
					
					/* テンポ */
					}else if(buf[j] == 'T'){
						j++;
						length=0;
						while( isdigit((int)buf[j]) ){
							length = length*10+buf[j]-'0';
							j++;
						}
						if( length<35 || 255<length ){
							printf("Parameter error of 'T' in:%d (%d)\n",line,length);
							exit(1);
						}
						fprintf(outfile, "\tdb\t\t$db,$%02x\t;ﾃﾝﾎﾟ\n",length);
					
					/* ヴォリューム0〜31 */
					}else if(buf[j] == 'V'){
						j++;
						length=0;
						while( isdigit((int)buf[j]) ){
							length = length*10+buf[j]-'0';
							j++;
						}
						if( length<0 || 31<length ){
							printf("Parameter error of 'V' in:%d (%d)\n",line,length);
							exit(1);
						}
						fprintf(outfile, "\tdb\t\t$dc,$%02x\t;ﾎﾞﾘｭｰﾑ\n",(char)length);
					
					/* パンポット0x00〜0xFF */
					}else if(buf[j] == 'P'){
						j++;
						pan_r=0;
						pan_l=0;
						while( isdigit((int)buf[j]) ){
							pan_r = pan_r*10+buf[j]-'0';
							j++;
						}
						if(buf[j]!=','){
							printf("Parameter error of 'P' in:%d (%d)\n",line,length);
							exit(1);
						}
						j++;
						while( isdigit((int)buf[j]) ){
							pan_l = pan_l*10+buf[j]-'0';
							j++;
						}
						if( (pan_r<0 || 15<pan_r)||(pan_l<0 || 15<pan_l) ){
							printf("Parameter error of 'P' in:%d (%d)\n",line,length);
							exit(1);
						}
						fprintf(outfile, "\tdb\t\t$dd,$%1x%1x\t;ﾊﾟﾝﾎﾟｯﾄ\n",pan_r,pan_l);
					
					/* 音長比1〜8 */
					}else if(buf[j] == 'Q'){
						j++;
						length=0;
						while( isdigit((int)buf[j]) ){
							length = length*10+buf[j]-'0';
							j++;
						}
						if( length<1 || 8<length ){
							printf("Parameter error of 'Q' in:%d (%d)\n",line,length);
							exit(1);
						}
						fprintf(outfile, "\tdb\t\t$de,$%02x\t;ｵﾝﾁｮｳﾋ\n",(char)length);
					/* 相対ヴォリューム */
					
					/* ダルセーニョ */
					
					/* セーニョ */
					
					/* リピートビギン */
					}else if(buf[j] == '['){
						j++;
						length=0;
						while( isdigit((int)buf[j]) ){
							length = length*10+buf[j]-'0';
							j++;
						}
						if( length<0 || 255<length ){
							printf("Parameter error of '[' in:%d (%d)\n",line,length);
							exit(1);
						}
						fprintf(outfile, "\tdb\t\t$e3,$%02x\t;ﾘﾋﾟｰﾄﾋﾞｷﾞﾝ\n",(char)length);
					
					/* リピートエンド */
					}else if(buf[j] == ']'){
						j++;
						fputs("\tdb\t\t$e4    \t;ﾘﾋﾟｰﾄｴﾝﾄﾞ\n", outfile);
					
					/* ウェーブ(音色) */
					}else if( (buf[j] == '@') && isdigit(buf[j+1]) ){
						j++;
						length=0;
						if( !isdigit((int)buf[j]) ){
							printf("Parameter error of '@' in:%d (null)\n",line);
							exit(1);
						}
						while( isdigit((int)buf[j]) ){
							length = length*10+buf[j]-'0';
							j++;
						}
						if( length<0 || 44<length ){
							printf("Parameter error of '@' in:%d (%d)\n",line,length);
							exit(1);
						}
						fprintf(outfile, "\tdb\t\t$e5,$%02x\t;ｵﾝｼｮｸ\n",(char)length);
					
					/* エンベロープ */
					}else if( (buf[j] == '@') && (buf[j+1] == 'E') ){
						j++; j++;
						length=0;
						while( isdigit((int)buf[j]) ){
							length = length*10+buf[j]-'0';
							j++;
						}
						if( length<0 || 127<length ){
							printf("Parameter error of '@E' in:%d (%d)\n",line,length);
							exit(1);
						}
						fprintf(outfile, "\tdb\t\t$e6,$%02x\t;ｴﾝﾍﾞﾛｰﾌﾟ\n",(char)length);
					
					/* 周波数変調 */
					/* FMディレイ */
					/* FM補正 */
					/* ピッチエンベロープ(PE) */
					/* PEディレイ */
					/* デチューン */
					}else if( (buf[j] == '@') && (buf[j+1] == 'D') ){
						j++; j++;
						length=0;
						put_length=1;
						if(buf[j]=='-'){
							/* マイナスの値 */
							put_length=-1;
							j++;
						}
						while( isdigit((int)buf[j]) ){
							length = length*10+buf[j]-'0';
							j++;
						}
						if( length<0 || 128<length ){
							printf("Parameter error of '@D' in:%d (%d)\n",line,length);
							exit(1);
						}
						put_length*=length;
						fprintf(outfile, "\tdb\t\t$ec,$%02x\t;ﾃﾞﾁｭｰﾝ\n",(char)put_length);
					
					/* スイープ */
					/* スイープタイム */
					/* ジャンプ */
					}else if(buf[j] == '/'){
						j++;
						k=6;
						strcpy(buf2, "LABEL_");
						while( buf[j] != '/' ){
							buf2[k]=buf[j];
							j++;
							k++;
						}
						j++;
						buf2[k]='\0';
						if( k==6 ){
							printf("Label name error in:%d\n",line);
							exit(1);
						}
						
						fprintf(outfile, "\tdb\t\t$ef    \t;ｼﾞｬﾝﾌﾟ\n");
						fprintf(outfile, "\tdw\t\t%s\t;ｼﾞｬﾝﾌﾟ\n",buf2);
					
					
					/* コール */
					}else if(buf[j] == '('){
						j++;
						k=6;
						strcpy(buf2, "LABEL_");
						while( buf[j] != ')' ){
							buf2[k]=buf[j];
							j++;
							k++;
						}
						j++;
						buf2[k]='\0';
						if( k==6 ){
							printf("Label name error in:%d\n",line);
							exit(1);
						}
						
						fprintf(outfile, "\tdb\t\t$f0    \t;ｺｰﾙ\n");
						fprintf(outfile, "\tdw\t\t%s\t;ｺｰﾙ\n",buf2);
					
					/* リターン */
					}else if(buf[j] == '\''){
						j++;
						fputs("\tdb\t\t$f1    \t;ﾘﾀｰﾝ\n", outfile);
					/* 移調 */
					/* 相対移調 */
					/* 全体移調 */
					/* ヴォリュームチェンジ */
					/* パンライトチェンジ */
					/* パンレフトチェンジ */
					/* モード */
					}else if( (buf[j] == '@') && (buf[j+1] == 'M') ){
						j++; j++;
						length=0;
						while( isdigit((int)buf[j]) ){
							length = length*10+buf[j]-'0';
							j++;
						}
						if( length<0 || 2<length ){
							printf("Parameter error of '@M' in:%d (%d)\n",line,length);
							exit(1);
						}
						pc_mode=length;
						fprintf(outfile, "\tdb\t\t$f8,$%02x\t;ﾓｰﾄﾞ\n",(char)length);
					
					/* フェードアウト */
					
					/* データエンド */
					}else if(buf[j] == '*'){
						j++;
						fputs("\tdb\t\t$ff    \t;ﾃﾞｰﾀｴﾝﾄﾞ\n", outfile);
					
					/* ここからは独自のコマンド */
					/* Ｌコマンド */
					}else if(buf[j] == 'L'){
						j++;
						length=0;
						while( isdigit((int)buf[j]) ){
							length = length*10+buf[j]-'0';
							j++;
						}
						switch(length){
							case 1:
							case 2:
							case 3:
							case 4:
							case 6:
							case 8:
							case 12:
							case 16:
							case 24:
							case 32:
							case 48:
							case 64:
							case 96:
								break;
							default:
								printf("Parameter error of 'L' in:%d (%d)\n",line,length);
								exit(1);
						}
						ch_length[channel]=length;
						
					/* 拡張ヴォリューム @V0〜128 */
					}else if( (buf[j] == '@') && (buf[j+1] == 'V') ){
						j++; j++;
						length=0;
						while( isdigit((int)buf[j]) ){
							length = length*10+buf[j]-'0';
							j++;
						}
						if( length<0 || 127<length ){
							printf("Parameter error of '@V' in:%d (%d)\n",line,length);
							exit(1);
						}
						put_length=length*31/127;
						fprintf(outfile, "\tdb\t\t$dc,$%02x\t;ﾎﾞﾘｭｰﾑ(@V)\n",(char)put_length);
					
					/* パーカッションモードの音符 */
					}else if( (k=instr("RBSMCH      ", buf[j])) != 0 && pc_mode==1 ){
						j++;
						while( !isdigit(buf[j]) ){
							j++;
						}
						if(buf[j]=='!' ){
							/* ヴォリューム強調 */
							j++;
						}
						/* 音長 */
						length=0;
						while( isdigit((int)buf[j]) ){
							length = length*10+buf[j]-'0';
							j++;
						}
						switch(length){
							case 0:
								length=ch_length[channel];
								break;
							case 1:
							case 2:
							case 3:
							case 4:
							case 6:
							case 8:
							case 12:
							case 16:
							case 24:
							case 32:
							case 48:
							case 64:
							case 96:
								break;
							default:
								printf("Length error in:%d(%d)\n",line,length);
								exit(1);
						}
						tone=k;
						if(k==0)tone=12;
						if(k==13)tone=1;
						tone*=16;
						/* ダイレクトレングスモード */
						put_length=(int)( (192+amari[channel])/length);
						amari[channel]=(int)( (192+amari[channel])%length);
						/* 符点処理 */
						futen=1.0;
						length=1;
						while(buf[j]=='.'){
							futen += pow(0.5, length++);
							j++;
						}
						put_length*=futen;
						fprintf(outfile, "\tdb\t\t$%02x,$%02x\t;ｵﾝﾌﾟ(ﾊﾟｰｶｯｼｮﾝ)\n", tone, put_length);
						/* 音長終わり */
					
					/* エラー */
					}else{
						printf("Not Support [%c]in:%d\n",buf[j],line);
						exit(1);
					}
					
				}/* MML展開ループ終わり */
				break;
		}/* switch(char)i)の終わり */
	}/* 読み込みがEOF */
	fclose(infile);
	/* トラックデータ先アドレス登録テーブル */
	if(f_channel==OFF){
		puts("No channel data.\n");
		exit(1);
	}
	fputs("track0:\n", outfile);
	for(a=0, i=1; i<=6; i++){
		a<<=1;
		a+=ch[i];
	}
	fprintf(outfile, "\tdb\t\t$%02x\t;00%1d%1d_%1d%1d%1d%1db\n",a,ch[1],ch[2],ch[3],ch[4],ch[5],ch[6]);
	for(i=1; i<=6; i++)
		if(ch[i]==ON)
			fprintf(outfile, "\tdw\t\tSTART_CH%1d\n",i);
	
#ifndef osx	/* damn BSD */
	fcloseall();
#endif
	return 0;
}



char *fgetstr( char *buffer,FILE *infile ,char *terminater)
{
	int i,c;

	i=0;
	while( (c=fgetc(infile))!=EOF && instr(terminater,(char)c)==0 ){
		if( instr(" \t", (char)c) == 0 ){  /* 空白とタブを取る */
			buffer[i]=(char)toupper(c);
			i++;
		}
	}
	buffer[i]=0;
	/* ;コメントがあった場合行末まで読み捨て。 */
	if( c==';' ){
		while( fgetc(infile)!='\n' ){
		}
	}
	return(buffer);
}

int instr( char* base, char obj )
{
	int i;
	for( i=0;i<strlen(base); i++ )
		if( base[i]==obj)
			return(i+1);
	return(0);
}

char *remove_ext(char* string)
{
	int i;
	char *name=(char *)malloc(9);
	for(i=0 ; (string[i]!='.')&&(i<11) ; i++)
		name[i]=string[i];
	name[i]='\0';
	return(name);
}
