To Do List:
~~~~~~~~~~~

1. Key MM should have NO autorepeat
2. Write-back storing of PW when MM +/-
3. Start in AUTO should use PW=0
4. WatchDog
5. Secure EEPROM Read/Write









-------------------
#define		aaa -500
#define		bbb -300

TEST:
			ldi a,low(aaa)
			mov dl,a
			ldi a,high(aaa)
			mov dh,a
			ldi al,low(bbb)
			ldi ah,high(bbb)

