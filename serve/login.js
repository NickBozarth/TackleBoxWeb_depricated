const loginForm = document.getElementById("login-form");
const signupForm = document.getElementById("signup-form");
const loginSignupHeader = document.querySelector("div#login-signup>h3");



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

    static err(value) {
        return new Result(value, false);
    }
}

async function textTosha256B64(text) {
    const textBuffer = new TextEncoder().encode(text);
    const textHash = await window.crypto.subtle.digest("SHA-256", textBuffer);
    const textHashB64 = btoa(String.fromCharCode(...new Uint8Array(textHash)));
    return textHashB64
}

function isValidUsername(username) {
    if (!username) return false;
    return /^[a-zA-Z0-9]+$/.test(username) && username.length > 7;
}

function containsValidPasswordChars(password) {
    return /^[a-zA-Z0-9\~\!\@\#\$\%\^\&\*\(\)\-\_\=\+\[\{\}\]\\\|\;\:\'\"\,\<\.\>\/\?]+$/.test(password);
}

function isAlphanumeric(string) { return /^[a-zA-Z0-9]+$/.test(string); }
function containsSymbol(string) { return /\W/.test(string); }
function containsAlphaLower(string) { return /[a-z]/.test(string); }
function containsAlphaUpper(string) { return /[A-Z]/.test(string); }

function retrieveUsername(form) {
    const usernameField = form.querySelector("input[name=username]");
    if (usernameField === null) return Result.err("No username field found");
    const username = usernameField.value;
    return Result.ok(username);
}

function retrieveUsernameValidated(form) {
    const usernameRes = retrieveUsername(form);
    if (usernameRes.is_err) return usernameRes;
    const username = usernameRes.value;

    if (username.length < 8) return Result.err("Username must be at least 8 characters long");
    if (!isAlphanumeric(username)) return Result.err("Username must be alphanumeric");
    return usernameRes;
}


function retrievePassword(form) {
    const passwordField = form.querySelector("input[name=password]");
    if (passwordField === null) return Result.err("No password field found");
    const password = passwordField.value;
    return Result.ok(password);
}

function retrievePasswordValidated(form) {
    const passwordRes = retrievePassword(form);
    if (passwordRes.is_err) return passwordRes;
    const password = passwordRes.value;

    if (password.length < 8) return Result.err("Password must be at lease 8 characters long");
    if (!containsValidPasswordChars(password)) return Result.err("Password contains invalid characters");
    if (!containsSymbol(password)) return Result.err("Password must contain at least one symbol");
    if (!containsAlphaLower(password)) return Result.err("Password must contain at least one lowercase character");
    if (!containsAlphaUpper(password)) return Result.err("Password must contain at least one capital character");

    return passwordRes;
}

async function processSignup() {
    // retrieve un & pw
    const usernameRes = retrieveUsernameValidated(signupForm);
    if (usernameRes.is_err) return usernameRes;
    const username = usernameRes.value;
    const passwordRes = retrievePasswordValidated(signupForm);
    if (passwordRes.is_err) return passwordRes;
    const password = passwordRes.value;

    // hash necessary data
    const usernameHashB64 = await textTosha256B64(username);
    const loginHashB64 = await textTosha256B64(username + password);

    // TODO parse success and fail values
    // send hashes
    return Result.ok(
        await fetchEncrypted(
            "/signup",
            {
                username_hash: usernameHashB64,
                login_hash: loginHashB64
            }
        )
    );
}

async function processLogin() {
    const usernameRes = retrieveUsername(loginForm);
    if (usernameRes.is_err) return usernameRes;
    const username = usernameRes.value;
    const passwordRes = retrievePassword(loginForm);
    if (passwordRes.is_err) return passwordRes;
    const password = passwordRes.value;

    // hash necessary data
    const usernameHashB64 = await textTosha256B64(username);
    const loginHashB64 = await textTosha256B64(username + password);

    // get the first response from the server
    // data will be structured "[login_id] [saltb64]"
    let resp1 = await fetchEncrypted(
        "/login",
        {
            username_hash: usernameHashB64,
        }
    );

    try {
        resp1 = JSON.parse(resp1);
        console.log("RESP1");
        console.log(resp1);
    } catch {
        return Result.err(resp1);
    }

    const login_hash2 = await textTosha256B64(loginHashB64 + resp1.salt);
    console.log(login_hash2);

    let resp2 = await fetchEncrypted(
        "/login",
        {
            request_id: resp1.request_id,
            username_hash: usernameHashB64,
            login_hash2: login_hash2,
        }
    )
    console.log(resp2);
}


loginForm.addEventListener("submit", async event => {
    event.preventDefault();

    const button = event.submitter;
    if (button.classList.contains("toggle-form")) {
        loginSignupHeader.innerHTML = "Signup";
        loginForm.hidden = true;
        signupForm.hidden = false;
        return;
    }

    console.log("what")

    console.log(await processLogin());
});

signupForm.addEventListener("submit", async event => {
    event.preventDefault();

    const button = event.submitter;
    if (button.classList.contains("toggle-form")) {
        loginSignupHeader.innerHTML = "Login";
        loginForm.hidden = false;
        signupForm.hidden = true;
        return;
    }

    console.log(await processSignup());
});