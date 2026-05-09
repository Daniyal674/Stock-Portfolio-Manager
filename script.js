let allTransactions = [];
let filteredTransactions = [];
let currentPage = 1;
const itemsPerPage = 10;
let reportsLimit = 6;
let undoStack = [];
let redoStack = [];

const STOCK_BETAS = {
    'AAPL': 1.1, 'TSLA': 2.4, 'MSFT': 0.9, 'GOOGL': 1.0, 'GOOG': 1.0, 'AMZN': 1.15,
    'META': 1.25, 'NFLX': 1.3, 'NVDA': 1.75, 'AMD': 1.6, 'INTC': 1.05, 'PYPL': 1.4,
    'BRK.B': 0.85, 'V': 0.95, 'MA': 1.0, 'BTC': 2.8, 'ETH': 3.2
};

function getStockBeta(name) {
    const ticker = name.toUpperCase().trim();
    return STOCK_BETAS[ticker] || 1.0;
}

window.switchTab = switchTab;
window.openModal = openModal;
window.handleSave = handleSave;
window.deleteTransaction = deleteTransaction;
window.editTransaction = editTransaction;
window.changePage = changePage;
window.openResetModal = openResetModal;
window.executeReset = executeReset;
window.exportCSV = exportCSV;
window.triggerImport = triggerImport;
window.handleFileImport = handleFileImport;
window.toggleSplitMode = toggleSplitMode;
window.undo = undo;
window.redo = redo;

document.addEventListener('DOMContentLoaded', () => {
    init();
    setupListeners();
});

async function init() {
    await fetchData(true); // Initial fetch
    updateDate();
    applyTheme();
}

async function fetchData(isInitial = false) {
    try {
        const res = await fetch('/api/transactions');
        allTransactions = await res.json();
        filteredTransactions = [...allTransactions];
        renderAll();
        updateHistoryButtons();
    } catch (e) { console.error('Fetch failed', e); }
}

function transactionsToCSV(data) {
    let csv = "ID,Date,Stock Name,Type,Quantity,Unit Price,Total Price\n";
    data.forEach(t => { csv += `${t.id},${t.date},${t.name},${t.type},${t.qty},${t.price},${t.total}\n`; });
    return csv;
}

async function pushState() {
    const currentState = transactionsToCSV(allTransactions);
    undoStack.push(currentState);
    redoStack = []; // Clear redo on new action
}

async function undo() {
    if (undoStack.length === 0) return;
    const currentState = transactionsToCSV(allTransactions);
    redoStack.push(currentState);
    const prevState = undoStack.pop();
    await applyState(prevState);
}

async function redo() {
    if (redoStack.length === 0) return;
    const currentState = transactionsToCSV(allTransactions);
    undoStack.push(currentState);
    const nextState = redoStack.pop();
    await applyState(nextState);
}

async function applyState(csvContent) {
    const res = await fetch('/api/import', { method: 'POST', body: csvContent });
    if (res.ok) fetchData();
}

function updateHistoryButtons() {
    const undoBtn = document.getElementById('undo-btn');
    const redoBtn = document.getElementById('redo-btn');
    if (undoBtn) undoBtn.disabled = undoStack.length === 0;
    if (redoBtn) redoBtn.disabled = redoStack.length === 0;
}

function renderAll() {
    renderDashboard();
    renderTransactions();
    renderHoldings();
    renderReports();
    lucide.createIcons();
}

function setupListeners() {
    document.querySelectorAll('.nav-item').forEach(item => {
        item.addEventListener('click', (e) => {
            e.preventDefault();
            switchTab(item.dataset.tab);
        });
    });
    document.getElementById('theme-toggle').addEventListener('click', toggleTheme);
    document.getElementById('dashboard-search').addEventListener('input', e => handleSearch(e.target.value));
    document.getElementById('history-search').addEventListener('input', e => handleSearch(e.target.value));
    document.getElementById('reports-limit').addEventListener('change', e => {
        reportsLimit = parseInt(e.target.value);
        renderReports();
    });
    document.getElementById('close-modal').addEventListener('click', () => {
        document.getElementById('add-modal').classList.remove('active');
    });
    document.getElementById('close-reset-modal').addEventListener('click', () => {
        document.getElementById('reset-modal').classList.remove('active');
    });
    const deleteModal = document.getElementById('delete-modal');
    document.getElementById('close-delete-modal').addEventListener('click', () => {
        deleteModal.classList.remove('active');
    });
    document.getElementById('confirm-delete-btn').addEventListener('click', () => {
        const id = document.getElementById('delete-id').value;
        executeDelete(id);
        deleteModal.classList.remove('active');
    });
}

function handleSearch(query) {
    const q = query.toLowerCase();
    filteredTransactions = allTransactions.filter(t => 
        t.name.toLowerCase().includes(q) || t.id.toString().includes(q)
    );
    currentPage = 1;
    renderAll();
}

function changePage(delta) {
    const maxPage = Math.ceil(filteredTransactions.length / itemsPerPage) || 1;
    currentPage += delta;
    if (currentPage < 1) currentPage = 1;
    if (currentPage > maxPage) currentPage = maxPage;
    renderAll();
}

function toggleSplitMode() {
    const type = document.getElementById('stock-type').value;
    const qtyLabel = document.getElementById('label-qty');
    const priceGroup = document.getElementById('price-group');
    if (type === 'Split') {
        qtyLabel.innerText = 'Split Ratio (e.g. 2 for 2:1)';
        priceGroup.style.display = 'none';
        document.getElementById('stock-price').value = '0';
    } else {
        qtyLabel.innerText = 'Quantity';
        priceGroup.style.display = 'block';
    }
}

function openModal(transaction = null) {
    const modal = document.getElementById('add-modal');
    const title = document.getElementById('modal-title');
    const form = document.getElementById('transaction-form');
    if (transaction) {
        title.innerText = 'Edit Transaction';
        document.getElementById('edit-id').value = transaction.id;
        document.getElementById('stock-name').value = transaction.name;
        document.getElementById('stock-date').value = transaction.date;
        document.getElementById('stock-type').value = transaction.type;
        document.getElementById('stock-qty').value = transaction.qty;
        document.getElementById('stock-price').value = transaction.price;
    } else {
        title.innerText = 'Add Transaction';
        form.reset();
        document.getElementById('edit-id').value = '';
    }
    toggleSplitMode();
    modal.classList.add('active');
}

async function handleSave() {
    const id = document.getElementById('edit-id').value;
    const name = document.getElementById('stock-name').value;
    const date = document.getElementById('stock-date').value;
    const type = document.getElementById('stock-type').value;
    const qty = parseFloat(document.getElementById('stock-qty').value);
    const price = parseFloat(document.getElementById('stock-price').value);
    if (!name || !date || isNaN(qty) || isNaN(price)) { alert('Please fill all fields.'); return; }
    
    await pushState(); // Capture for undo

    const total = type === 'Split' ? 0 : qty * price;
    const finalId = id || 0;
    const csvLine = `${finalId},${date},${name},${type},${qty},${price},${total}`;
    if (id) await fetch(`/api/transactions?id=${id}`, { method: 'DELETE' });
    const res = await fetch('/api/transactions', { method: 'POST', body: csvLine });
    if (res.ok) { document.getElementById('add-modal').classList.remove('active'); fetchData(); }
}

async function deleteTransaction(id) {
    document.getElementById('delete-id').value = id;
    document.getElementById('delete-modal').classList.add('active');
}

async function executeDelete(id) {
    await pushState(); // Capture for undo
    await fetch(`/api/transactions?id=${id}`, { method: 'DELETE' });
    fetchData();
}

function openResetModal() {
    document.getElementById('reset-modal').classList.add('active');
}

async function executeReset() {
    await pushState(); // Capture for undo
    const res = await fetch('/api/reset', { method: 'POST' });
    if (res.ok) { document.getElementById('reset-modal').classList.remove('active'); fetchData(); }
}

async function exportCSV() {
    const res = await fetch('/api/export');
    if (res.ok) {
        const blob = await res.blob();
        const url = window.URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `transactions_${new Date().toISOString().split('T')[0]}.csv`;
        document.body.appendChild(a);
        a.click();
        window.URL.revokeObjectURL(url);
    }
}

function triggerImport() { document.getElementById('import-input').click(); }

async function handleFileImport(event) {
    const file = event.target.files[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = async (e) => {
        await pushState(); // Capture for undo
        const content = e.target.result;
        const res = await fetch('/api/import', { method: 'POST', body: content });
        if (res.ok) { alert('Data imported successfully!'); fetchData(); } else { alert('Error importing data.'); }
    };
    reader.readAsText(file);
    event.target.value = '';
}

function calculateProfessionalHoldings() {
    const holdings = {};
    let totalPortfolioValue = 0;
    let realizedPnL = 0;
    let weightedBetaSum = 0;
    const stockStats = {};
    allTransactions.sort((a,b) => new Date(a.date) - new Date(b.date)).forEach(t => {
        if (!holdings[t.name]) holdings[t.name] = { qty: 0, avgCost: 0, currentVal: 0, beta: getStockBeta(t.name) };
        if (!stockStats[t.name]) stockStats[t.name] = { invested: 0, withdrawn: 0, realizedPnL: 0 };
        const h = holdings[t.name]; const s = stockStats[t.name];
        if (t.type === 'Buy') {
            const newQty = h.qty + t.qty;
            h.avgCost = ((h.avgCost * h.qty) + t.total) / newQty;
            h.qty = newQty;
            s.invested += t.total;
        } else if (t.type === 'Sell') {
            const pnl = (t.price - h.avgCost) * t.qty;
            realizedPnL += pnl; s.realizedPnL += pnl; s.withdrawn += t.total;
            h.qty -= t.qty; if (h.qty <= 0) { h.qty = 0; h.avgCost = 0; }
        } else if (t.type === 'Split') {
            if (h.qty > 0) { h.qty *= t.qty; h.avgCost /= t.qty; }
        }
    });
    Object.values(holdings).forEach(h => { if (h.qty > 0) { h.currentVal = h.qty * h.avgCost; totalPortfolioValue += h.currentVal; } });
    Object.values(holdings).forEach(h => { if (h.qty > 0) { const weight = h.currentVal / totalPortfolioValue; weightedBetaSum += weight * h.beta; } });
    return { holdings, totalPortfolioValue, realizedPnL, portfolioBeta: weightedBetaSum, stockStats };
}

function renderDashboard() {
    const { totalPortfolioValue, realizedPnL, portfolioBeta } = calculateProfessionalHoldings();
    const valEl = document.getElementById('total-value');
    const pnlEl = document.getElementById('net-pnl');
    const betaEl = document.getElementById('portfolio-beta');
    const betaLabel = document.getElementById('beta-label');
    const tradesEl = document.getElementById('total-trades');
    if (valEl) valEl.innerText = `Rs. ${Math.round(totalPortfolioValue).toLocaleString()}`;
    if (pnlEl) { pnlEl.innerText = `Rs. ${Math.round(realizedPnL).toLocaleString()}`; pnlEl.className = 'stat-value ' + (realizedPnL >= 0 ? 'status-up' : 'status-down'); }
    if (betaEl) {
        betaEl.innerText = portfolioBeta.toFixed(2);
        betaEl.className = 'stat-value ' + (portfolioBeta > 1.2 ? 'status-down' : (portfolioBeta < 0.8 ? 'status-up' : ''));
        if (betaLabel) betaLabel.innerText = portfolioBeta > 1.2 ? 'High Volatility' : (portfolioBeta < 0.8 ? 'Low Volatility' : 'Market Normal');
    }
    if (tradesEl) tradesEl.innerText = allTransactions.length;
    const body = document.querySelector('#recent-table tbody');
    if (body) {
        body.innerHTML = allTransactions.slice(-5).reverse().map(t => `<tr><td>${t.date}</td><td>${t.name}</td><td><span class="badge ${t.type.toLowerCase()}">${t.type}</span></td><td>${t.qty}</td><td>Rs. ${t.total.toLocaleString()}</td><td><button class="action-btn" onclick="deleteTransaction(${t.id})"><i data-lucide="trash-2"></i></button></td></tr>`).join('');
    }
}

function renderHoldings() {
    const { holdings, totalPortfolioValue } = calculateProfessionalHoldings();
    const container = document.getElementById('holdings-container');
    if (!container) return;
    const entries = Object.entries(holdings).filter(([_, data]) => data.qty > 0).sort((a,b) => b[1].currentVal - a[1].currentVal);
    if (entries.length === 0) { container.innerHTML = ""; return; }
    let totalPct = 0;
    const items = entries.map(([name, data]) => {
        const pct = (data.currentVal / totalPortfolioValue) * 100;
        totalPct += parseFloat(pct.toFixed(1));
        return { name, data, pct: parseFloat(pct.toFixed(1)) };
    });
    const diff = 100 - totalPct;
    if (items.length > 0) items[0].pct = parseFloat((items[0].pct + diff).toFixed(1));
    container.innerHTML = items.map(item => `
        <div class="stat-card glass holding-card">
            <div class="holding-header-row">
                <div class="holding-main"><p class="holding-name">${item.name}</p><p class="stat-label">${Math.round(item.data.qty)} Shares | Beta: ${item.data.beta}</p></div>
                <div class="holding-details"><p class="holding-avg">Avg: Rs. ${item.data.avgCost.toFixed(2)}</p><p class="holding-total">Val: Rs. ${Math.round(item.data.currentVal).toLocaleString()}</p></div>
            </div>
            <div class="allocation-bar-container">
                <div class="allocation-info"><span>Allocation</span><span>${item.pct.toFixed(1)}%</span></div>
                <div class="allocation-bar"><div class="allocation-fill" style="width: ${item.pct}%"></div></div>
            </div>
        </div>
    `).join('');
}

function renderReports() {
    const { stockStats } = calculateProfessionalHoldings();
    const body = document.querySelector('#reports-table tbody');
    if (body) {
        const sortedStats = Object.entries(stockStats).sort((a,b) => b[1].realizedPnL - a[1].realizedPnL);
        const limitedStats = sortedStats.slice(0, reportsLimit);
        body.innerHTML = limitedStats.map(([name, s]) => {
            const roi = s.invested > 0 ? (s.realizedPnL / s.invested) * 100 : 0;
            return `<tr><td><strong>${name}</strong></td><td>Rs. ${Math.round(s.invested).toLocaleString()}</td><td>Rs. ${Math.round(s.withdrawn).toLocaleString()}</td><td class="${s.realizedPnL >= 0 ? 'status-up' : 'status-down'}">Rs. ${Math.round(s.realizedPnL).toLocaleString()}</td><td class="${roi >= 0 ? 'status-up' : 'status-down'}">${roi.toFixed(2)}%</td><td><span class="badge ${s.realizedPnL >= 0 ? 'buy' : 'sell'}">${s.realizedPnL >= 0 ? 'PROFIT' : 'LOSS'}</span></td></tr>`;
        }).join('');
    }
}

function renderTransactions() {
    const body = document.querySelector('#full-history-table tbody');
    if (!body) return;
    const start = (currentPage - 1) * itemsPerPage;
    const end = start + itemsPerPage;
    const pageItems = [...filteredTransactions].reverse().slice(start, end);
    body.innerHTML = pageItems.map(t => `<tr><td>#${t.id}</td><td>${t.date}</td><td><strong>${t.name}</strong></td><td><span class="badge ${t.type.toLowerCase()}">${t.type}</span></td><td>${t.qty}</td><td>${t.price}</td><td>${t.total}</td><td><button class="action-btn" onclick="editTransaction(${t.id})"><i data-lucide="edit-3"></i></button><button class="action-btn delete-btn" onclick="deleteTransaction(${t.id})"><i data-lucide="trash-2"></i></button></td></tr>`).join('');
    const maxPage = Math.ceil(filteredTransactions.length / itemsPerPage) || 1;
    const info = document.getElementById('page-info');
    if (info) info.innerText = `Page ${currentPage} of ${maxPage}`;
}

function editTransaction(id) { const t = allTransactions.find(x => x.id === id); if (t) openModal(t); }
function switchTab(id) {
    document.querySelectorAll('.nav-item').forEach(i => i.classList.toggle('active', i.dataset.tab === id));
    document.querySelectorAll('.tab-content').forEach(c => c.classList.toggle('active', c.id === `${id}-tab`));
}
function toggleTheme() {
    const isDark = document.body.classList.toggle('light-theme');
    localStorage.setItem('theme', isDark ? 'light' : 'dark');
    document.getElementById('theme-icon').setAttribute('data-lucide', isDark ? 'sun' : 'moon');
    lucide.createIcons();
}
function applyTheme() {
    const theme = localStorage.getItem('theme');
    if (theme === 'light') { document.body.classList.add('light-theme'); document.getElementById('theme-icon').setAttribute('data-lucide', 'sun'); }
}
function updateDate() {
    const options = { weekday: 'long', year: 'numeric', month: 'long', day: 'numeric' };
    const el = document.getElementById('current-date');
    if (el) el.innerText = new Date().toLocaleDateString('en-US', options);
}
