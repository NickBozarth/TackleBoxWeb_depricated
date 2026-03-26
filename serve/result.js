class Result {
    constructor(value, is_ok) {
        this.value = value;
        this.is_ok = is_ok;
    }

    get is_err() {
        return !this.is_ok;
    }


    static ok(value) {
        return new Result(value, true);
    }

    static err(value, err) {
        if (err === undefined) return new Result(value, false);
        return new Result(value + ": " + err, false);
    }
}