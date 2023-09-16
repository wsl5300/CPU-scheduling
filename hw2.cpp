# include <stdio.h>
# include <stdlib.h>
# include <iostream>
# include <fstream>
# include <vector>
# include <queue>
# include <map>


using namespace std ;

class CPU_Sort {
  
  struct DATA {
    int id ;
    int burst ;
    int now = 0 ;
    int arr_time ;
    int priority ;
  };
  
  struct gDATA {
    int time ; // 時刻
    char id ;
  };
  
  int t_slice ;
  vector <DATA> buffer ; // 輸入以arr排序
  vector <gDATA> Gantt[6] ;
  
  map <int, map<int, int> > Wait ; // 收集1-6方法的等待時間 (方法 id time )
  map <int, int> wait ; // 紀錄單獨等待時間
  
  map <int, map<int, int> > T_around ; // 收集1-6方法的turnaround (方法 id time )
  map <int, int> t_around ; // 紀錄單獨turnaround
  
  float average = 0 ;
  float T = 0 ;
  
  public :
    
    bool loading ( string &filename, int &command ) {
      char t[256] ;
      DATA data ;
      ifstream file ;
      cin >> filename ;
      filename = filename + ".txt" ;
      file.open( filename.c_str(), ios::in ) ;
      if ( !file.is_open()) {
        cout << "unfound file" << endl ;
        return false ;
      } // if
    
      file >> command ;
      file >> t_slice ;
      file >> t >> t >> t >> t >> t >> t ;
      //cout << t ;
      while (file >> data.id >> data.burst >> data.arr_time >> data.priority) {
        buffer.push_back( data ) ;
      } // while

      file.close() ;
      
      Sort_buffer() ;
      
/*
      cout << buffer.size() << "*" << endl ;

      for ( int i = 0 ; i < buffer.size() ; i++ ) {
        cout << buffer[i].id  << "\t" << buffer[i].burst << "\t" ;
        cout << buffer[i].arr_time << "\t" << buffer[i].priority << "\t" ;
        cout << endl ;
      }// for
*/
      return true ;
    } // load()
  
    void swap( int a, int b ) {
      DATA temp ;
      temp = buffer[a] ;
      buffer[a] = buffer[b] ;
      buffer[b] = temp ;
      return ;
      } // swap()

    void Sort_buffer() { // buffer依照  arr_time  id  排序

      for ( int i = buffer.size() - 1 ; i >= 0 ; i-- ) {
        for( int j = 0; j <= i-1 ; j++ ){
          
          if ( buffer[j].arr_time > buffer[j+1].arr_time ) swap( j, j+1 ) ;

          else if ( buffer[j].arr_time == buffer[j+1].arr_time )
            if ( buffer[j].id > buffer[j+1].id ) swap( j, j+1 ) ;
            
        } // for
      } // for

    } // BubbleSort()
  
    void gantt( int time, int id, int num ) { // 放入甘特圖
      gDATA temp ;
      temp.time = time ;
      if ( id > 9 ) temp.id = (char) id - 9 + 64 ;
      else if ( id == -1 ) temp.id = '-' ;
      else temp.id = (char) id + 48 ;
      Gantt[num].push_back(temp) ;
    } // gantt()
    
    void waiting( int id, int burst ) { // turnaround - burst
      wait[id] = t_around[id] - burst ;
      average = average + wait[id] ;
    } // waiting()
    
    void t_arounding( int time, int id, int arr ) { // finish - arr
      t_around[id] = time-arr ;
      T = T + t_around[id] ;
    } // t_arounding()
  
    void FCFS () { // First Come First Serve
      int time = 0 ;
      average = 0 ;
      T = 0 ;

      for ( int i = 0 ; i < buffer.size() ; i++ ) {
        if ( time >= buffer[i].arr_time ) {
          for ( int m = 0 ; m < buffer[i].burst ; m++ ) {
            gantt( time, buffer[i].id, 0 ) ;
            time++ ;
          } // for
          
          t_arounding( time, buffer[i].id, buffer[i].arr_time ) ;
          waiting( buffer[i].id, buffer[i].burst ) ;
          
        } // if
        else {
          for ( ; time < buffer[i].arr_time ; time++ ) gantt( time, -1, 0 ) ;
          i-- ;
        } // else
        
      } // for
      
      average = (float)average/buffer.size() ;
      cout << endl << "FCFS average Waiting time : " << average ;
      
      cout << endl << "FCFS average Turnaround time : " << T << endl << endl ;
      
      T_around[0] = t_around ;
      t_around.clear() ;
      Wait[0] = wait ;
      wait.clear() ;
      
      
      /*
      cout << endl ;
      for ( int i = 0 ; i < Gantt[0].size() ; i++ ) cout << Gantt[0][i].id ;
      cout << endl << endl << "///////////////////////////" << endl ;
      for ( const auto& i : wait ) cout << i.first << "\t" << i.second << endl ;
      cout << endl << "///////////////////////////" << endl ;
      for ( const auto& i : t_around ) cout << i.first << "\t" << i.second << endl ;
      */
    } // FCFS()
  
    void RR () { // Round Robin
      average = 0 ;
      T = 0 ;
      int time = 0 ;
      int i = 0 ;
      queue <DATA> buf ;
      DATA cpu ;

      
      for ( ; !buf.empty() || time < buffer[buffer.size()-1].arr_time ; ) {
        
        if ( buf.empty() ) for ( ; time < buffer[i].arr_time ; time++ ) gantt( time, -1, 1 ) ;
        
        for ( ; i < buffer.size() && buffer[i].arr_time <= time ; i++ ) buf.push( buffer[i] ) ;
        
        cpu = buf.front() ;
        buf.pop() ;
        
        for ( int m = 0 ; m < t_slice && cpu.burst != cpu.now ; m++ ) { // 使用cpu
          gantt( time, cpu.id, 1 ) ;
          cpu.now++ ;
          time++ ;
        } // for
        
        for ( ; i < buffer.size() && buffer[i].arr_time <= time ; i++ ) buf.push( buffer[i] ) ;
        
        if ( cpu.burst > cpu.now ) { // 還沒做完再進入佇列
          buf.push(cpu) ;
        } // if
        else {
          t_arounding( time, cpu.id, cpu.arr_time ) ;
          waiting( cpu.id, cpu.burst ) ;
        } // else
        
      } // for
      
      average = (float)average/buffer.size() ;
      cout << endl << "RR average Waiting time : " << average ;
      
      cout << endl << "RR average Turnaround time : " << T << endl << endl ;

      T_around[1] = t_around ;
      t_around.clear() ;
      Wait[1] = wait ;
      wait.clear() ;

/*
      cout << endl ;
      for ( int i = 0 ; i < Gantt[1].size() ; i++ ) cout << Gantt[1][i].id ;
      cout << endl << endl << "///////////////////////////" << endl ;
      for ( const auto& i : wait ) cout << i.first << "\t" << i.second << endl ;
      cout << endl << "///////////////////////////" << endl ;
      for ( const auto& i : t_around ) cout << i.first << "\t" << i.second << endl ;
      */
      
    } // RR()
  
    void SJF () { // Shortest Job First
      average = 0 ;
      T = 0 ;
      int time = 0 ;
      int i = 0 ;
      vector<DATA> buf ;
      auto p = buf.begin() ;
      
      for ( ; !buf.empty() || time < buffer[buffer.size()-1].arr_time ; ) {

        if ( !buf.empty() ) {
          for ( auto cpu = buf.begin() ; cpu != buf.end() ; cpu = buf.begin() ) {
            for ( int m = 0 ; m < cpu->burst ; m++ ) {
              gantt( time, cpu->id, 2 ) ;
              time++ ;
            } // for
            
            t_arounding( time, cpu->id, cpu->arr_time ) ;
            waiting( cpu->id, cpu->burst ) ;
            
            buf.erase( cpu ) ;
            
            for ( ; i < buffer.size() && buffer[i].arr_time <= time ; i++ ) {
              for ( p = buf.begin() ; p != buf.end() && p->burst <= buffer[i].burst ; p++ ) ;
                buf.insert( p, buffer[i] ) ;
            } // for

          } // for
          
          
        } // if
        
        if ( i >= buffer.size() ) break ;
        
        for ( ; time < buffer[i].arr_time ; time++ ) gantt( time, -1, 2 ) ;
        
        for ( ; i < buffer.size() && buffer[i].arr_time <= time ; i++ ) {
          for ( p = buf.begin() ; p != buf.end() && p->burst <= buffer[i].burst ; p++ ) ;
          buf.insert( p, buffer[i] ) ;
        } // for
        
      } // for
      
      average = (float)average/buffer.size() ;
      cout << endl << "SJF average Waiting time : " << average ;
      
      cout << endl << "SJF average Turnaround time : " << T << endl << endl ;

      T_around[2] = t_around ;
      t_around.clear() ;
      Wait[2] = wait ;
      wait.clear() ;

/*
      cout << endl ;
      for ( int i = 0 ; i < Gantt[2].size() ; i++ ) cout << Gantt[2][i].id ;
      cout << endl << endl << "///////////////////////////" << endl ;
      for ( const auto& i : wait ) cout << i.first << "\t" << i.second << endl ;
      cout << endl << "///////////////////////////" << endl ;
      for ( const auto& i : t_around ) cout << i.first << "\t" << i.second << endl ;
*/
      
    } // SJF()
  
    void SRTF () { // Shortest Remaining Time First
      average = 0 ;
      T = 0 ;
      int time = 0 ;
      int i = 0 ;
      vector <DATA> buf ;
      auto p = buf.begin() ;
      DATA cpu ;
      DATA temp ;
      
      for ( ; time < buffer[buffer.size()-1].arr_time || !buf.empty() ; ) {
        if ( buf.empty() ) {
          for ( ; i < buffer.size() && time < buffer[i].arr_time ; time++ ) gantt( time, -1, 3 ) ;
          
          for ( ; i < buffer.size() && buffer[i].arr_time <= time ; i++ ) {
            for ( p = buf.begin() ; p != buf.end() && p->burst <= buffer[i].burst ; p++ ) ;
            buf.insert( p, buffer[i] ) ;
          } // for
        } // if
        
        p = buf.begin() ;
        cpu = *p ;
        buf.erase( p ) ;

        for ( ; cpu.now < cpu.burst ; time++ ) {
          if ( i < buffer.size() && buffer[i].arr_time <= time ) {
            for ( ; i < buffer.size() && buffer[i].arr_time == time ; i++ ) {
              for ( p = buf.begin() ; p != buf.end() && p->burst <= buffer[i].burst ; p++ ) ;
              buf.insert( p, buffer[i] ) ;
            } // for

            p = buf.begin() ;

            if ( cpu.burst - cpu.now > p->burst ) {
              temp = *p ;
              buf.erase( p ) ;
              buf.insert( p, cpu ) ;
              cpu = temp ;
            } // if
          } // if
          
          gantt( time, cpu.id, 3 ) ;
          cpu.now++ ;
        } // for
        
        t_arounding( time, cpu.id, cpu.arr_time ) ;
        waiting( cpu.id, cpu.burst ) ;
        
      } // for
      
      average = (float)average/buffer.size() ;
      cout << endl << "SRTF average Waiting time : " << average ;
      
      cout << endl << "SRTF average Turnaround time : " << T << endl << endl ;
      
      T_around[3] = t_around ;
      t_around.clear() ;
      Wait[3] = wait ;
      wait.clear() ;
      /*
      cout << endl ;
      for ( int i = 0 ; i < Gantt[1].size() ; i++ ) cout << Gantt[3][i].id ;
      cout << endl << endl << "///////////////////////////" << endl ;
      for ( const auto& i : wait ) cout << i.first << "\t" << i.second << endl ;
      cout << endl << "///////////////////////////" << endl ;
      for ( const auto& i : t_around ) cout << i.first << "\t" << i.second << endl ;
*/

    } // SRTF()
  
    void HRRN () { // Highest Response Ratio Next
      average = 0 ;
      T = 0 ;
      int time = 0 ;
      int i = 0 ;
      vector <DATA> buf ;
      auto p = buf.begin() ;
      auto p1 = p ;
      float temp = 0 ;
      DATA cpu ;
      
      for ( ; !buf.empty() || i < buffer.size() ; ) {
        if ( buf.empty() ) for ( ; i < buffer.size() && time < buffer[i].arr_time ; time++ ) gantt( time, -1, 4 ) ;
        
        for ( ; i < buffer.size() && buffer[i].arr_time <= time ; i++ ) buf.push_back( buffer[i] ) ;

        for ( p = buf.begin() ; p != buf.end() ; p++ ) {
          if ( p == buf.begin() || (float)( time - p->arr_time + p->burst ) / p->burst > temp ) {
            temp = (float)( time - p->arr_time + p->burst ) / p->burst ;
            p1 = p ;
          } // if
        } // for
        
        cpu = *p1 ;
        buf.erase( p1 ) ;
        
        for ( int m = 0 ; m < cpu.burst ; m++ ) {
          gantt( time, cpu.id, 4 ) ;
          time++ ;
        } // for
        
        t_arounding( time, cpu.id, cpu.arr_time ) ;
        waiting( cpu.id, cpu.burst ) ;
        
      } // for
      
      average = (float)average/buffer.size() ;
      cout << endl << "HRRN average Waiting time : " << average ;
      
      cout << endl << "HRRN average Turnaround time : " << T << endl << endl ;

      T_around[4] = t_around ;
      t_around.clear() ;
      Wait[4] = wait ;
      wait.clear() ;
/*
      cout << endl ;
      for ( int i = 0 ; i < Gantt[4].size() ; i++ ) cout << Gantt[4][i].id ;
      cout << endl << endl << "///////////////////////////" << endl ;
      for ( const auto& i : wait ) cout << i.first << "\t" << i.second << endl ;
      cout << endl << "///////////////////////////" << endl ;
      for ( const auto& i : t_around ) cout << i.first << "\t" << i.second << endl ;
*/
    } // HRRN()
  
    void PPRR () { // Preemptive Priority + RR
      average = 0 ;
      T = 0 ;
      int time = 0 ;
      int i = 0 ;
      vector <DATA> buf ;
      auto p = buf.begin() ;
      DATA cpu ;
      
      for ( ; !buf.empty() || time < buffer[buffer.size()-1].arr_time ; ) {
        
        if ( buf.empty() ) {
          for ( ; time < buffer[i].arr_time ; time++ ) gantt( time, -1, 5 ) ;
          for ( ; i < buffer.size() && buffer[i].arr_time <= time ; i++ ) {
            for ( p = buf.begin() ; p != buf.end() && p->priority <= buffer[i].priority ; p++ ) ;
            buf.insert( p, buffer[i] ) ;
          } // for
          
        } // if
        
        
        //for ( int i = 0 ; i < buf.size() ; i++ ) cout << buf[i].id<< " " ;
        //cout << endl ;
        cpu = *buf.begin() ;
        buf.erase( buf.begin() ) ;


        for ( int m = 0 ; m < t_slice && cpu.burst != cpu.now ; m++ ) { // 使用cpu
          gantt( time, cpu.id, 5 ) ;
          cpu.now++ ;
          time++ ;
          // 若有priority更小的process出現
          if ( i < buffer.size() && time == buffer[i].arr_time && buffer[i].priority < cpu.priority ) break ;

        } // for
        
        for ( ; i < buffer.size() && buffer[i].arr_time <= time ; i++ ) {
            for ( p = buf.begin() ; p != buf.end() && p->priority <= buffer[i].priority ; p++ ) ;
            buf.insert( p, buffer[i] ) ;
        } // for


        if ( cpu.burst > cpu.now ) { // cpu還沒做完再進入佇列

          for ( p = buf.begin() ; p != buf.end() && p->priority <= cpu.priority ; p++ ) ;
          buf.insert( p, cpu ) ;
        } // if
        else {
          t_arounding( time, cpu.id, cpu.arr_time ) ;
          waiting( cpu.id, cpu.burst ) ;
        } // else

      } // for
      
      average = (float)average/buffer.size() ;
      cout << endl << "PPRR average Waiting time : " << average ;
      
      cout << endl << "PPRR average Turnaround time : " << T << endl << endl ;

      T_around[5] = t_around ;
      t_around.clear() ;
      Wait[5] = wait ;
      wait.clear() ;
/*
      cout << endl ;
      for ( int i = 0 ; i < Gantt[5].size() ; i++ ) cout << Gantt[5][i].id ;
      cout << endl << endl << "///////////////////////////" << endl ;
      for ( const auto& i : wait ) cout << i.first << "\t" << i.second << endl ;
      cout << endl << "///////////////////////////" << endl ;
      for ( const auto& i : t_around ) cout << i.first << "\t" << i.second << endl ;
*/
    } // PPRR()
  

    void print ( string &filename, int &command ) {
      filename =  "out_" + filename ;
	    ofstream out ( filename.c_str(), ios::out|ios::trunc ) ;

	    if (out.is_open()) {
	      if ( command == 1 ) out << "FCFS\n" ;
	      else if ( command == 2 ) out << "RR" ;
	      else if ( command == 3 ) out << "SJF" ;
	      else if ( command == 4 ) out << "SRTF" ;
	      else if ( command == 5 ) out << "HRRN" ;
	      else if ( command == 6 ) out << "PPRR" ;
	      else if ( command == 7 ) out << "All" ;
	      
        if ( command == 1 || command == 7 ) {
          out << "\n==        FCFS==\n" ;
          for ( int i = 0 ; i < Gantt[0].size() ; i++ ) out << Gantt[0][i].id ;
        } // if
        if ( command == 2 || command == 7 ) {
          out << "\n==          RR==\n" ;
          for ( int i = 0 ; i < Gantt[1].size() ; i++ ) out << Gantt[1][i].id ;
        } // if
        if ( command == 3 || command == 7 ) {
          out << "\n==         SJF==\n" ;
          for ( int i = 0 ; i < Gantt[2].size() ; i++ ) out << Gantt[2][i].id ;
        } // if
        if ( command == 4 || command == 7 ) {
          out << "\n==        SRTF==\n" ;
          for ( int i = 0 ; i < Gantt[3].size() ; i++ ) out << Gantt[3][i].id ;
        } // if
        if ( command == 5 || command == 7 ) {
          out << "\n==        HRRN==\n" ;
          for ( int i = 0 ; i < Gantt[4].size() ; i++ ) out << Gantt[4][i].id ;
        } // if
        if ( command == 6 || command == 7 ) {
          out << "\n==        PPRR==\n" ;
          for ( int i = 0 ; i < Gantt[5].size() ; i++ ) out << Gantt[5][i].id ;
        } // if
        
        out << "\n===========================================================\n\nWaiting Time\nID" ;
        if ( command == 1 || command == 7 ) out << "\tFCFS" ;
        if ( command == 2 || command == 7 ) out << "\tRR" ;
        if ( command == 3 || command == 7 ) out << "\tSJF" ;
        if ( command == 4 || command == 7 ) out << "\tSRTF" ;
        if ( command == 5 || command == 7 ) out << "\tHRRN" ;
        if ( command == 6 || command == 7 ) out << "\tPPRR" ;
        out << "\n===========================================================\n" ;
        
        for ( int i = 0 ; i < buffer.size() ; i++ ) {
          if ( command == 7 ) out << Wait[0].begin()->first ;
          else out << Wait[command-1].begin()->first ;
          for ( int j = 1 ; j < 7 ; j++ ) {
            if ( command == j || command == 7 ) {
              out << "\t" << Wait[j-1].begin()->second ;
              Wait[j-1].erase(Wait[j-1].begin()) ;
            } // if
          } // for
          out << endl ;
        } // for
        
        out << "===========================================================\n\nTurnaround Time\nID" ;
        if ( command == 1 || command == 7 ) out << "\tFCFS" ;
        if ( command == 2 || command == 7 ) out << "\tRR" ;
        if ( command == 3 || command == 7 ) out << "\tSJF" ;
        if ( command == 4 || command == 7 ) out << "\tSRTF" ;
        if ( command == 5 || command == 7 ) out << "\tHRRN" ;
        if ( command == 6 || command == 7 ) out << "\tPPRR" ;
        out << "\n===========================================================\n" ;
        
        for ( int i = 0 ; i < buffer.size() ; i++ ) {
          if ( command == 7 ) out << T_around[0].begin()->first ;
          else out << T_around[command-1].begin()->first ;
          for ( int j = 1 ; j < 7 ; j++ ) {
            if ( command == j || command == 7 ) {
              out << "\t" << T_around[j-1].begin()->second ;
              T_around[j-1].erase(T_around[j-1].begin()) ;
            } // if
          } // for
          out << endl ;
        } // for
        out << "===========================================================" ;
        
        
        
        out.close(); // 關閉檔案
      } // if
      else {
        cout << "Unable to open file\n";
      } // else
      
      buffer.clear() ;
      for ( int i = 0 ; i < 7 ; i++ ) Gantt[i].clear() ;
      Wait.clear() ;
      T_around.clear() ;
    } // print()
  
};



int main() {
  int command ;
  CPU_Sort test ;
  string filename ;
  
  while( true ) {
		cout << "*** OS HomeWork 02 : Sort          **" << endl ;
		cout << "* 0. QUIT                          *" << endl ;
		cout << "* 1. First Come First Serve        *" << endl ;
		cout << "* 2. Round Robin                   *" << endl ;
		cout << "* 3. Shortest Job First            *" << endl ;
		cout << "* 4. Shortest Remaining Time First *" << endl ;
		cout << "* 5. Highest Response Ratio Next   *" << endl ;
		cout << "* 6. Preemptive Priority + RR      *" << endl ;
		cout << "* 7. All                           *" << endl ;
		cout << "*************************************" << endl ;
		cout << "filename : " ;
		
    if ( !test.loading( filename, command ) ) {
      cout << "Please operate again.\n\n" ;
      continue ;
    } // if

    if ( command > 7 || command < 0 ) cout << "Command out of range !" << endl << endl ;
 	  else if ( command == 0 ) break ;
    else if ( command == 1 ) test.FCFS() ;
    else if ( command == 2 ) test.RR() ;
	  else if ( command == 3 ) test.SJF() ;
	  else if ( command == 4 ) test.SRTF() ;
	  else if ( command == 5 ) test.HRRN() ;
	  else if ( command == 6 ) test.PPRR() ;
	  else if ( command == 7 ) {
	    test.FCFS() ;
	    test.RR() ;
	    test.SJF() ;
	    test.SRTF() ;
	    test.HRRN() ;
	    test.PPRR() ;
    } // else if
	  
	  test.print( filename, command ) ;

	  cout << endl << endl ;

	} // while
} // main()
