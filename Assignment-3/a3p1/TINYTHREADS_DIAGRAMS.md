# TinyThreads Visual Diagrams and Flow Charts

This document contains Mermaid diagrams that can be rendered in GitHub, GitLab, or using tools like [Mermaid Live Editor](https://mermaid.live).

---

## 1. System Architecture Overview

```mermaid
graph TB
    subgraph "TinyThreads System Architecture"
        A[Main Program] -->|spawn| B[Thread Spawning]
        B -->|allocate| C{freeQ}
        C -->|configure| D[Thread Block]
        D -->|enqueue| E[readyQ]
        
        E -->|dispatch| F[Current Thread]
        F -->|yield| E
        F -->|complete| G[freeQ]
        
        subgraph "Thread Pool"
            T0[Thread 0<br/>1KB stack]
            T1[Thread 1<br/>1KB stack]
            T2[Thread 2<br/>1KB stack]
            T3[Thread 3<br/>1KB stack]
            T4[Thread 4<br/>1KB stack]
        end
        
        subgraph "Queues"
            C
            E
            G
        end
    end
    
    style F fill:#90EE90
    style C fill:#FFB6C6
    style E fill:#87CEEB
    style G fill:#DDA0DD
```

---

## 2. Thread State Machine

```mermaid
stateDiagram-v2
    [*] --> FREE: System Init
    FREE --> READY: spawn()
    READY --> RUNNING: dispatch()
    RUNNING --> READY: yield()
    RUNNING --> FREE: function returns
    
    note right of FREE
        Thread available
        in freeQ
    end note
    
    note right of READY
        Waiting in readyQ
        for CPU time
    end note
    
    note right of RUNNING
        Pointed by 'current'
        Executing on CPU
    end note
```

---

## 3. Context Switch Sequence

```mermaid
sequenceDiagram
    participant MA as Main Thread
    participant YF as yield()
    participant RQ as readyQ
    participant T0 as Thread 0
    
    Note over MA: Executing computePrimes(1)
    MA->>YF: call yield()
    YF->>YF: DISABLE interrupts
    YF->>RQ: p = dequeue()
    RQ-->>YF: Thread 0
    YF->>RQ: enqueue(Main)
    YF->>MA: setjmp(Main->context)
    Note over MA: Save CPU state
    YF->>T0: longjmp(T0->context)
    Note over T0: Restore CPU state
    T0->>T0: ENABLE interrupts
    Note over T0: Executing computePower(0)
    T0->>YF: call yield()
    YF->>YF: DISABLE interrupts
    YF->>RQ: p = dequeue()
    RQ-->>YF: Main Thread
    YF->>RQ: enqueue(Thread 0)
    YF->>T0: setjmp(T0->context)
    Note over T0: Save CPU state
    YF->>MA: longjmp(Main->context)
    Note over MA: Restore CPU state
    MA->>MA: ENABLE interrupts
    Note over MA: Continue execution
```

---

## 4. spawn() Function Flowchart

```mermaid
flowchart TD
    A[spawn function, arg] --> B[DISABLE Interrupts]
    B --> C{initialized?}
    C -->|No| D[initializeThreads]
    D --> E[Initialize all 5 threads]
    E --> F[Setup freeQ linked list]
    F --> G[Set initialized = 1]
    G --> H[dequeue from freeQ]
    C -->|Yes| H
    
    H --> I[Configure Thread:<br/>function = f<br/>arg = a]
    I --> J{setjmp newp-context == 1?}
    
    J -->|Yes: Thread Starts| K[ENABLE Interrupts]
    K --> L[Execute: function arg]
    L --> M[Function Returns]
    M --> N[enqueue current to freeQ]
    N --> O[dispatch next from readyQ]
    
    J -->|No: First Time| P[SETSTACK newp]
    P --> Q[enqueue newp to readyQ]
    Q --> R[ENABLE Interrupts]
    R --> S[Return to caller]
    
    style J fill:#FFD700
    style K fill:#90EE90
    style P fill:#87CEEB
```

---

## 5. yield() Function Flowchart

```mermaid
flowchart TD
    A[Thread calls yield] --> B[DISABLE Interrupts]
    B --> C{readyQ != NULL?}
    C -->|No| D[No other threads<br/>Continue running]
    D --> E[ENABLE Interrupts]
    E --> F[Return]
    
    C -->|Yes| G[p = dequeue readyQ]
    G --> H[enqueue current to readyQ]
    H --> I[dispatch p]
    
    I --> J{setjmp current-context == 0?}
    J -->|Yes: Save succeeded| K[current = p]
    K --> L[longjmp p-context, 1]
    L --> M[CPU jumps to Thread p]
    
    J -->|No: Resumed from longjmp| N[ENABLE Interrupts]
    N --> F
    
    style C fill:#FFD700
    style J fill:#FFB6C6
    style M fill:#90EE90
```

---

## 6. Queue Operations

```mermaid
flowchart LR
    subgraph "enqueue operation"
        A1[New Thread] --> B1{Queue empty?}
        B1 -->|Yes| C1[queue = thread<br/>thread->next = NULL]
        B1 -->|No| D1[Traverse to tail]
        D1 --> E1[tail->next = thread<br/>thread->next = NULL]
    end
    
    subgraph "dequeue operation"
        A2[Queue Head] --> B2{Queue empty?}
        B2 -->|Yes| C2[Return NULL]
        B2 -->|No| D2[p = queue]
        D2 --> E2[queue = queue->next]
        E2 --> F2[Return p]
    end
```

---

## 7. Memory Layout

```mermaid
graph TB
    subgraph "Global Memory Space"
        subgraph "Thread Pool Array"
            T0["threads[0]<br/>────────<br/>idx: 0<br/>function: ptr<br/>arg: int<br/>next: ptr<br/>context: jmp_buf<br/>stack[1024]<br/>deadline: uint"]
            T1["threads[1]<br/>────────<br/>idx: 1<br/>function: ptr<br/>arg: int<br/>next: ptr<br/>context: jmp_buf<br/>stack[1024]<br/>deadline: uint"]
            T2["threads[2]<br/>..."]
            T3["threads[3]<br/>..."]
            T4["threads[4]<br/>..."]
        end
        
        subgraph "Queue Pointers"
            FQ["freeQ<br/>────<br/>thread*"]
            RQ["readyQ<br/>────<br/>thread*"]
            DQ["doneQ<br/>────<br/>thread*"]
            CU["current<br/>────<br/>thread*"]
        end
        
        subgraph "Init Thread"
            IT["initp<br/>────────<br/>idx: -1<br/>Main thread"]
        end
    end
    
    FQ -.->|points to| T2
    RQ -.->|points to| T0
    CU -.->|points to| IT
    
    T0 -.->|next| T1
    T2 -.->|next| T3
    T3 -.->|next| T4
    
    style T0 fill:#90EE90
    style IT fill:#FFD700
    style CU fill:#FF6B6B
```

---

## 8. Program Execution Timeline

```mermaid
gantt
    title TinyThreads Execution Timeline
    dateFormat X
    axisFormat %L ms
    
    section Main Thread
    Init System           :0, 100
    spawn(computePower)   :100, 50
    computePrimes starts  :150, 200
    Prime 2 found         :350, 500
    yield()              :850, 100
    Wait for turn        :950, 500
    Prime 3 found        :1450, 500
    yield()             :1950, 100
    
    section Thread 0
    Created              :100, 50
    Wait in readyQ       :150, 700
    0^2=0 computed       :850, 500
    yield()             :1350, 100
    Wait in readyQ      :1450, 500
    1^2=1 computed      :1950, 500
    
    section Context
    Main → T0 switch     :850, 100
    T0 → Main switch    :1350, 100
    Main → T0 switch    :1950, 100
```

---

## 9. Concurrency Model

```mermaid
graph LR
    subgraph "Cooperative Multithreading"
        A[Thread A Running] -->|Voluntary yield| B[Save Context]
        B --> C[Update Queues]
        C --> D[Restore Context]
        D --> E[Thread B Running]
        E -->|Voluntary yield| F[Save Context]
        F --> G[Update Queues]
        G --> H[Restore Context]
        H --> A
    end
    
    subgraph "Critical Section Protection"
        I[Enter Critical] --> J[DISABLE Interrupts<br/>cpsid i]
        J --> K[Manipulate Queues]
        K --> L[ENABLE Interrupts<br/>cpsie i]
        L --> M[Exit Critical]
    end
    
    style A fill:#90EE90
    style E fill:#87CEEB
    style J fill:#FFB6C6
    style L fill:#90EE90
```

---

## 10. Initialization Sequence

```mermaid
sequenceDiagram
    participant M as main()
    participant I as initializeThreads()
    participant S as spawn()
    participant T as threads[0-4]
    participant Q as Queues
    
    M->>M: piface_init()
    M->>S: spawn(computePower, 0)
    S->>S: Check initialized flag
    S->>I: initializeThreads()
    
    loop For each thread 0-4
        I->>T: Set idx, function=NULL, arg=-1
        I->>T: Set next pointer
        I->>T: Set Period_Deadline = INT_MAX
    end
    
    I->>Q: freeQ = &threads[0]
    I->>Q: threads[4].next = NULL
    I->>S: Return (initialized=1)
    
    S->>Q: newp = dequeue(freeQ)
    S->>T: Configure threads[0]
    S->>T: setjmp(threads[0].context)
    S->>T: SETSTACK(threads[0])
    S->>Q: enqueue(threads[0], readyQ)
    S->>M: Return
    
    M->>M: computePrimes(1) starts
    
    Note over M,Q: System ready for multithreading
```

---

## 11. Data Structure Relationships

```mermaid
classDiagram
    class thread_block {
        +short idx
        +void (*function)(int)
        +int arg
        +thread next
        +jmp_buf context
        +char stack[1024]
        +unsigned int Period_Deadline
        +unsigned int Rel_Period_Deadline
    }
    
    class mutex_block {
        +int locked
        +thread waitQ
    }
    
    class Queues {
        +thread freeQ
        +thread readyQ
        +thread doneQ
        +thread current
    }
    
    thread_block "5" --* "1" Queues : managed by
    mutex_block "1" --> "0..*" thread_block : waitQ
    Queues --> thread_block : points to
    
    note for thread_block "Each thread has 1KB private stack"
    note for mutex_block "For future synchronization"
    note for Queues "Global queue management"
```

---

## 12. Round-Robin Scheduling

```mermaid
graph TB
    subgraph "Round-Robin Cycle"
        A[Main: Prime 2] -->|yield| B[Context Switch]
        B --> C[Thread 0: 0^2=0]
        C -->|yield| D[Context Switch]
        D --> E[Main: Prime 3]
        E -->|yield| F[Context Switch]
        F --> G[Thread 0: 1^2=1]
        G -->|yield| H[Context Switch]
        H --> I[Main: Prime 5]
        I -->|yield| J[Context Switch]
        J --> K[Thread 0: 2^2=4]
        K -->|yield| L[Context Switch]
        L --> A
    end
    
    style A fill:#FFD700
    style C fill:#87CEEB
    style E fill:#FFD700
    style G fill:#87CEEB
    style I fill:#FFD700
    style K fill:#87CEEB
```

---

## 13. Critical Section Timing

```mermaid
sequenceDiagram
    participant T as Thread
    participant CPU as CPU/Interrupts
    participant Q as Queue Operations
    
    Note over T: Normal Execution
    T->>CPU: DISABLE() - cpsid i
    activate CPU
    Note over CPU: Interrupts Blocked
    
    T->>Q: dequeue(readyQ)
    Q-->>T: Thread pointer
    T->>Q: enqueue(current, readyQ)
    T->>Q: Update current pointer
    
    T->>CPU: ENABLE() - cpsie i
    deactivate CPU
    Note over CPU: Interrupts Enabled
    Note over T: Normal Execution
```

---

## 14. Assignment 3 Part 1 Specific Flow

```mermaid
flowchart TD
    A[System Boot] --> B[piface_init]
    B --> C[Display: DT8025 - A3P1]
    C --> D[Wait 2 seconds]
    D --> E[Clear Display]
    E --> F[spawn computePower, 0]
    
    F --> G[Thread 0 created<br/>in readyQ]
    G --> H[Main: computePrimes 1]
    
    H --> I{is_prime n?}
    I -->|No| J[n++]
    J --> I
    I -->|Yes| K[Display T1: Prime n]
    K --> L[Wait 0.5s]
    L --> M[yield]
    
    M --> N[Switch to Thread 0]
    N --> O[Compute n^2]
    O --> P[Display T0: n^2=result]
    P --> Q[Wait 0.5s]
    Q --> R[yield]
    R --> S[Switch to Main]
    S --> J
    
    style F fill:#90EE90
    style M fill:#FFB6C6
    style N fill:#87CEEB
    style R fill:#FFB6C6
    style S fill:#FFD700
```

---

## 15. Complete System Interaction

```mermaid
graph TD
    subgraph "Hardware Layer"
        ARM[ARMv8 CPU]
        INT[Interrupt Controller]
        TIM[System Timer]
        LCD[PiFace LCD]
    end
    
    subgraph "TinyThreads Library"
        CTX[Context Switch<br/>setjmp/longjmp]
        QUEUE[Queue Management]
        SCHED[Scheduler]
        CRIT[Critical Sections<br/>DISABLE/ENABLE]
    end
    
    subgraph "Application Layer"
        MAIN[Main Thread<br/>computePrimes]
        T0[Thread 0<br/>computePower]
    end
    
    ARM --> CTX
    INT --> CRIT
    TIM --> MAIN
    TIM --> T0
    
    CTX --> SCHED
    QUEUE --> SCHED
    CRIT --> QUEUE
    
    SCHED --> MAIN
    SCHED --> T0
    
    MAIN --> LCD
    T0 --> LCD
    
    style ARM fill:#FFD700
    style CTX fill:#90EE90
    style QUEUE fill:#87CEEB
    style MAIN fill:#FFB6C6
    style T0 fill:#DDA0DD
```

---

## How to Use These Diagrams

### Online Rendering
1. Copy any diagram code block (including the \`\`\`mermaid markers)
2. Paste into [Mermaid Live Editor](https://mermaid.live)
3. The diagram will render interactively

### In Markdown Viewers
- **GitHub**: Automatically renders Mermaid diagrams
- **GitLab**: Automatically renders Mermaid diagrams
- **VS Code**: Install "Markdown Preview Mermaid Support" extension
- **Obsidian**: Built-in Mermaid support

### Export Options
- PNG/SVG: Use Mermaid Live Editor's export function
- PDF: Print from browser after rendering
- Presentations: Use Marp or reveal.js with Mermaid plugin

---

## Legend

- 🟢 **Green**: Active/Running state
- 🔵 **Blue**: Ready/Waiting state
- 🔴 **Red**: Critical section/Protected
- 🟡 **Yellow**: Decision point/Main thread
- 🟣 **Purple**: Completed/Done

---

**Note**: These diagrams are designed to be progressive learning tools. Start with the simple architecture overview and progress to the detailed sequence diagrams to build a complete mental model of TinyThreads operation.

