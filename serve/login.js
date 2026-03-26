const loginForm  = document.getElementById("login-form");
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

function isValidUsername(username) {
    if (!username) return false;
    return /^[a-zA-Z0-9]+$/.test(username) && username.length > 7;
}

function containsValidPasswordChars(password) {
    return /^[a-zA-Z0-9\~\!\@\#\$\%\^\&\*\(\)\-\_\=\+\[\{\}\]\\\|\;\:\'\"\,\<\.\>\/\?]+$/.test(password);
}

function isAlphanumeric(string)     { return /^[a-zA-Z0-9]+$/.test(string); }
function containsSymbol(string)     { return /\W/.test(string); }
function containsAlphaLower(string) { return /[a-z]/.test(string); }
function containsAlphaUpper(string) { return /[A-Z]/.test(string); }

function retrieveUsername(form) {
    const usernameField = form.querySelector("input[name=username]");
    if (usernameField === null)    return Result.err("No username field found");
    const username = usernameField.value;    
    return Result.ok(username);
}

function retrieveUsernameValidated(form) {
    const usernameRes = retrieveUsername(form);
    if (usernameRes.is_err) return usernameRes;
    const username = usernameRes.value;

    if (username.length < 8)       return Result.err("Username must be at least 8 characters long");
    if (!isAlphanumeric(username)) return Result.err("Username must be alphanumeric");
    return usernameRes;
}


function retrievePassword(form) {
    const passwordField = form.querySelector("input[name=password]");
    if (passwordField === null)                return Result.err("No password field found");
    const password = passwordField.value;
    return Result.ok(password);
}

function retrievePasswordValidated(form) {
    const passwordRes = retrievePassword(form);
    if (passwordRes.is_err) return passwordRes;
    const password = passwordRes.value;

    if (password.length < 8)                   return Result.err("Password must be at lease 8 characters long");
    if (!containsValidPasswordChars(password)) return Result.err("Password contains invalid characters");
    if (!containsSymbol(password))             return Result.err("Password must contain at least one symbol");
    if (!containsAlphaLower(password))         return Result.err("Password must contain at least one lowercase character");
    if (!containsAlphaUpper(password))         return Result.err("Password must contain at least one capital character");

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

    // hash with un, pw
    const unpw = username + password;
    const unpwBuffer = new TextEncoder().encode(unpw);
    const loginHash = await window.crypto.subtle.digest("SHA-256", unpwBuffer);
    const loginHashB64 = btoa(String.fromCharCode(...new Uint8Array(loginHash)));
    console.log(loginHashB64);
    
    // send hash
    return Result.ok(await fetchEncrypted("/signup", { username: username }));
}


loginForm.addEventListener("submit", event => {
    event.preventDefault();

    const button = event.submitter;
    if (button.classList.contains("toggle-form")) {
        loginSignupHeader.innerHTML = "Signup";
        loginForm.hidden  = true;
        signupForm.hidden = false;
        return;
    }
});

signupForm.addEventListener("submit", async event => {
    event.preventDefault();

    const button = event.submitter;
    if (button.classList.contains("toggle-form")) {
        loginSignupHeader.innerHTML = "Login";
        loginForm.hidden  = false;
        signupForm.hidden = true;
        return;
    }

    console.log(await processSignup());
});