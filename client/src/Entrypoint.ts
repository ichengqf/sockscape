class SockContext {
    private static didInit: boolean = false;

    private static _masterSock: Connection;
    public static get masterSock(): Connection {
        return this._masterSock;
    }
    
    private static _slaveSock: Connection;
    public static get slaveSock(): Connection {
        return this._slaveSock;
    }

    public static init(): void {
        if(this.didInit) return;

        
    }
}

class Entrypoint {
    private static initStatus = {
        keyInit: false,
        fileCache: false
    };

    private static initCheck(): void {
        var done = true;
        for(var i in this.initStatus)
            done = done && this.initStatus[i];

        if(done)
            Entrypoint.ready();
    }

    public static start(): void {
        var conn = new Connection("ws://127.0.0.1:8080", []);
        conn.open();

        Key.init(() => {
            this.initStatus.keyInit = true;
            this.initCheck();
        });

        FileCache.initCache(
            // SUCCESS 
            () => { 
                this.initStatus.fileCache = true;
                this.initCheck();
            },

            // FAILURE
            (error: string) => { 
                CriticalStop.redirect(error);
            } 
        ); 
    }

    private static ready(): void {
        //alert("ready");
    }
}