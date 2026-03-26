const loginForm  = document.getElementById("login-form");
const signupForm = document.getElementById("signup-form");
const loginSignupHeader = document.querySelector("div#login-signup>h3");



class FormResult {
    constructor(value, is_ok) {
        this.value = value;
        this.is_ok = is_ok;
    }

    get is_err() {
        return !this.is_ok;
    }


    static ok(value) {
        return new FormResult(value, true);
    }

    static err(value) {
        return new FormResult(value, false);
    }
}

function isValidUsername(username) {
    if (!username) return false;
    return /^[a-zA-Z0-9]+$/.test(username) && username.length > 7;
}

function containsValidPasswordChars(password) {
    return /^[a-zA-Z0-9\~\!\@\#\$\%\^\&\*\(\)\-\_\=\+\[\{\}\]\\\|\;\:\'\"\,\<\.\>\/\?]+$/.test(password);
}

function isAlphanumeric(string)     { return /^[a-zA-Z0-9]+$/.test(username); }
function containsSymbol(string)     { return /\W/.test(string); }
function containsAlphaLower(string) { return /[a-z]/.test(string); }
function containsAlphaUpper(string) { return /[A-Z]/.test(string); }

function retrieveUsername(form) {
    const usernameField = form.querySelector("input[name=username]");
    if (usernameField === null)    return FormResult.err("No username field found");
    const username = usernameField.value;    
    return FormResult.ok(username);
}

function retrieveUsernameValidated(form) {
    const usernameRes = retrieveUsername(form);
    if (usernameRes.is_err) return usernameRes;
    const username = usernameRes.value;

    if (username.length < 8)       return FormResult.err("Username must be at least 8 characters long");
    if (!isAlphanumeric(username)) return FormResult.err("Username must be alphanumeric");
    return usernameRes;
}


function retrievePassword(form) {
    const passwordField = form.querySelector("input[name=password]");
    if (passwordField === null)                return FormResult.err("No password field found");
    const password = passwordField.value;
    return FormResult.ok(password);
}

function retrievePasswordValidated(form) {
    const passwordRes = retrievePassword(form);
    if (passwordRes.is_err) return passwordRes;
    const password = passwordRes.value;

    if (password.length < 8)                   return FormResult.err("Password must be at lease 8 characters long");
    if (!containsValidPasswordChars(password)) return FormResult.err("Password contains invalid characters");
    if (!containsSymbol(password))             return FormResult.err("Password must contain at least one symbol");
    if (!containsAlphaLower(password))         return FormResult.err("Password must contain at least one lowercase character");
    if (!containsAlphaUpper(password))         return FormResult.err("Password must contain at least one capital character");

    return passwordRes;
}

function processSignup() {
    // retrieve un & pw
    const usernameRes = retrieveUsernameValidated(signupForm);
    if (usernameRes.is_err) return usernameRes;
    const username = usernameRes.value;
    const passwordRes = retrievePasswordValidated(signupForm);
    if (passwordRes.is_err) return passwordRes;
    const password = passwordRes.value;

    // generate iv
    // hash with iv, un, pw
    // send hash
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

signupForm.addEventListener("submit", event => {
    event.preventDefault();

    const button = event.submitter;
    if (button.classList.contains("toggle-form")) {
        loginSignupHeader.innerHTML = "Login";
        loginForm.hidden  = false;
        signupForm.hidden = true;
        return;
    }

});