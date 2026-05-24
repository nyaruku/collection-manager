'use strict';

let hideUnknownMaps = false;
const hiddenColumns = new Set();
const collectionCounts = {};
let detailSequence = 0;

const viewSessionKeys = {
    hideUnknownMaps: 'collection-manager.hideUnknownMaps',
    hiddenColumns: 'collection-manager.hiddenColumns',
};

const i18n = window.CM_I18N || {};

function tr(key, values = {}) {
    return (i18n[key] || key).replace(/\{(\w+)}/g, (_, name) =>
        Object.prototype.hasOwnProperty.call(values, name) ? values[name] : '{' + name + '}');
}

function restoreViewSession() {
    try {
        hideUnknownMaps = sessionStorage.getItem(viewSessionKeys.hideUnknownMaps) === '1';

        hiddenColumns.clear();
        const hideableColumns = new Set(columnDefs.filter(def => def.hideable).map(def => def.col));
        const savedColumns = JSON.parse(sessionStorage.getItem(viewSessionKeys.hiddenColumns) || '[]');
        if (Array.isArray(savedColumns)) {
            savedColumns.forEach(col => {
                if (hideableColumns.has(col)) hiddenColumns.add(col);
            });
        }
    } catch {
        hideUnknownMaps = false;
        hiddenColumns.clear();
    }
}

function saveViewSession() {
    try {
        sessionStorage.setItem(viewSessionKeys.hideUnknownMaps, hideUnknownMaps ? '1' : '0');
        sessionStorage.setItem(viewSessionKeys.hiddenColumns, JSON.stringify([...hiddenColumns]));
    } catch {
        // Ignore disabled/unavailable session storage.
    }
}

function updateHideUnknownLabel() {
    const item = document.getElementById('hide-unknown-item');
    if (item) item.textContent = tr(hideUnknownMaps ? 'menuViewShowUnknown' : 'menuViewHideUnknown');
}

// unknown-map visibility
const dynamicStyle = document.createElement('style');
document.head.appendChild(dynamicStyle);

function updateDynamicStyles() {
    const rules = [];
    hiddenColumns.forEach(col => rules.push(`[data-col="${col}"] { display: none !important; }`));
    if (hideUnknownMaps) rules.push('.beatmap-row[data-unknown] { display: none !important; }');
    dynamicStyle.textContent = rules.join('\n');
}

function setColumnVisible(col, visible) {
    if (!visible) hiddenColumns.add(col); else hiddenColumns.delete(col);
    saveViewSession();
    updateDynamicStyles();
}

function toggleHideUnknown() {
    hideUnknownMaps = !hideUnknownMaps;
    updateHideUnknownLabel();
    saveViewSession();
    updateDynamicStyles();
    rerenderVirtualLists();
}

const listData = {};
const beatmapSelections = {};
const beatmapSelectionAnchors = {};
let beatmapClipboard = null;

// Virtual renderer: keep all maps in JS, only render visible rows.
const VIRTUAL_OVERSCAN = 20;
const DEFAULT_ROW_STEP = 32;

function visibleBeatmaps(state) {
    return state.visible || state.sorted;
}

function refreshVisibleBeatmaps(state) {
    state.visible = hideUnknownMaps ? state.sorted.filter(beatmap => !!beatmap.title) : state.sorted;
}

function measureRowStep(state) {
    const row = state.rowsWindow.querySelector('.beatmap-row');
    if (!row) return state.rowStep || DEFAULT_ROW_STEP;

    const rowStyle = getComputedStyle(row);
    const height = row.getBoundingClientRect().height;
    const marginBottom = parseFloat(rowStyle.marginBottom) || 0;
    return Math.max(1, height + marginBottom);
}

function renderVirtualList(listId) {
    const state = listData[listId];
    if (!state || !state.list.isConnected || !state.scrollEl) return;

    const scrollEl = state.scrollEl;
    const items = visibleBeatmaps(state);
    state.rowStep = measureRowStep(state);

    state.body.style.height = (items.length * state.rowStep) + 'px';

    const scrollRect = scrollEl.getBoundingClientRect();
    const bodyRect = state.body.getBoundingClientRect();
    const firstVisible = Math.min(
        items.length,
        Math.floor(Math.max(0, scrollRect.top - bodyRect.top) / state.rowStep)
    );
    const visibleCount = Math.ceil(scrollEl.clientHeight / state.rowStep);
    const start = Math.max(0, firstVisible - VIRTUAL_OVERSCAN);
    const end = Math.min(items.length, firstVisible + visibleCount + VIRTUAL_OVERSCAN);

    state.rowsWindow.style.transform = 'translateY(' + (start * state.rowStep) + 'px)';

    const frag = document.createDocumentFragment();
    for (let i = start; i < end; i++) {
        frag.appendChild(buildBeatmapRow(items[i], state.mode, state.collectionName));
    }
    state.rowsWindow.replaceChildren(frag);
    state.renderedStart = start;
    state.renderedEnd = end;

    const measured = measureRowStep(state);
    if (Math.abs(measured - state.rowStep) > 0.5) {
        state.rowStep = measured;
        state.body.style.height = (items.length * state.rowStep) + 'px';
        state.rowsWindow.style.transform = 'translateY(' + (start * state.rowStep) + 'px)';
    }
}

function scheduleVirtualRender(listId) {
    const state = listData[listId];
    if (!state || state.renderScheduled) return;
    state.renderScheduled = true;
    requestAnimationFrame(() => {
        state.renderScheduled = false;
        renderVirtualList(listId);
    });
}

function rerenderVirtualLists() {
    Object.keys(listData).forEach(listId => {
        const state = listData[listId];
        refreshVisibleBeatmaps(state);
        if (state.list?.isConnected) scheduleVirtualRender(listId);
    });
}

function startRender(list, listId) {
    const state = listData[listId];
    refreshVisibleBeatmaps(state);
    state.rowsWindow.replaceChildren();
    renderVirtualList(listId);
}

window.addEventListener('resize', rerenderVirtualLists);

function selectionKey(mode, collectionName) {
    return mode + '\u0000' + collectionName;
}

function selectionFor(mode, collectionName) {
    const key = selectionKey(mode, collectionName);
    if (!beatmapSelections[key]) beatmapSelections[key] = new Set();
    return beatmapSelections[key];
}

function syncBeatmapSelection(list, mode, collectionName) {
    const selected = selectionFor(mode, collectionName);
    list.querySelectorAll('.beatmap-row').forEach(row => {
        row.classList.toggle('beatmap-row-selected', selected.has(row.dataset.md5));
    });
}

function selectedHashesFor(mode, collectionName, fallbackMd5) {
    const selected = selectionFor(mode, collectionName);
    return selected.has(fallbackMd5) ? [...selected] : [fallbackMd5];
}

function selectBeatmapRow(row, mode, collectionName, md5, additive) {
    const key = selectionKey(mode, collectionName);
    const selected = selectionFor(mode, collectionName);

    if (!additive) {
        selected.clear();
        row.closest('.beatmap-list')
            ?.querySelectorAll('.beatmap-row-selected')
            .forEach(el => el.classList.remove('beatmap-row-selected'));
    }

    if (additive && selected.has(md5)) {
        selected.delete(md5);
        row.classList.remove('beatmap-row-selected');
    } else {
        selected.add(md5);
        row.classList.add('beatmap-row-selected');
    }

    beatmapSelectionAnchors[key] = md5;
}

function selectBeatmapRange(row, mode, collectionName, md5, additive) {
    const list = row.closest('.beatmap-list');
    const state = listData[list?.id];
    const key = selectionKey(mode, collectionName);
    const anchor = beatmapSelectionAnchors[key];

    if (!list || !state || !anchor) {
        selectBeatmapRow(row, mode, collectionName, md5, additive);
        return;
    }

    const hashes = visibleBeatmaps(state).map(beatmap => beatmap.md5);
    const from = hashes.indexOf(anchor);
    const to = hashes.indexOf(md5);
    if (from < 0 || to < 0) {
        selectBeatmapRow(row, mode, collectionName, md5, additive);
        return;
    }

    const selected = selectionFor(mode, collectionName);
    if (!additive) selected.clear();

    const start = Math.min(from, to);
    const end = Math.max(from, to);
    for (let i = start; i <= end; i++) selected.add(hashes[i]);

    beatmapSelectionAnchors[key] = md5;
    syncBeatmapSelection(list, mode, collectionName);
}

function copyBeatmaps(mode, collectionName, hashes) {
    beatmapClipboard = {
        mode,
        collectionName,
        hashes: [...new Set(hashes)],
    };
    setStatus(tr('copiedBeatmapsFrom', { count: beatmapClipboard.hashes.length, name: collectionName }));
}

async function pasteBeatmaps(mode, targetCollection) {
    if (!beatmapClipboard || beatmapClipboard.hashes.length === 0) {
        setStatus(tr('clipboardEmpty'));
        return;
    }

    try {
        const result = await apiFetch('/api/collections/copy', {
            from: beatmapClipboard.mode,
            to: mode,
            collection: beatmapClipboard.collectionName,
            targetCollection,
            hashes: beatmapClipboard.hashes,
        });
        await loadCollections(mode);
        await selectCollectionByName(mode, targetCollection);
        setStatus(tr('pastedBeatmapsInto', { count: result.copied, name: targetCollection }));
    } catch (e) { setStatus(tr('pasteError', { error: e.message })); }
}

function setStatus(text) {
    document.getElementById('status-text').textContent = text;
}

function updateStatusCounts() {
    document.getElementById('status-counts').textContent =
        Object.entries(collectionCounts)
            .map(([mode, count]) => tr('collectionCount', { mode, count }))
            .join('  ');
}

function showAbout() {
    const dialog = document.getElementById('about-dialog');
    dialog.style.transform = 'none';
    dialog.classList.remove('d-none');
    dialog.style.left = Math.max(0, (window.innerWidth - dialog.offsetWidth) / 2) + 'px';
    dialog.style.top = Math.max(0, (window.innerHeight - dialog.offsetHeight) / 3) + 'px';
}

(function initAboutDrag() {
    const dialog = document.getElementById('about-dialog');
    const titlebar = document.getElementById('about-titlebar');
    titlebar.addEventListener('mousedown', function(event) {
        event.preventDefault();
        const rect = dialog.getBoundingClientRect();
        dialog.style.transform = 'none';
        dialog.style.left = rect.left + 'px';
        dialog.style.top = rect.top + 'px';
        const originX = event.clientX - rect.left;
        const originY = event.clientY - rect.top;
        document.onmousemove = function(e) {
            const maxX = window.innerWidth - dialog.offsetWidth;
            const maxY = window.innerHeight - dialog.offsetHeight;
            dialog.style.left = Math.min(Math.max(0, e.clientX - originX), maxX) + 'px';
            dialog.style.top = Math.min(Math.max(0, e.clientY - originY), maxY) + 'px';
        };
        document.onmouseup = function() {
            document.onmousemove = null;
            document.onmouseup = null;
        };
    });
})();

// Helpers
function escapeHtmlKeepLeadingSpaces(str) {
    return (str || '')
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/^ +/, spaces => '&nbsp;'.repeat(spaces.length));
}

function formatStars(rating) {
    return rating > 0 ? rating.toFixed(2) + '\u2605' : '';
}

function cloneTemplate(id) {
    return document.getElementById(id).content.cloneNode(true).firstElementChild;
}

function switchTab(mode) {
    const button = document.getElementById('tab-' + mode);
    if (button) bootstrap.Tab.getOrCreateInstance(button).show();
}

// Column definitions
const columnDefs = [
    { name: tr('columnTitle'), col: 'title', hideable: false },
    { name: tr('columnArtist'), col: 'artist', hideable: true },
    { name: tr('columnDifficulty'), col: 'difficulty', hideable: true },
    { name: tr('columnMapper'), col: 'mapper', hideable: true },
    { name: tr('columnStars'), col: 'stars', hideable: true },
    { name: tr('columnId'), col: 'id', hideable: true },
    { name: tr('columnSet'), col: 'setid', hideable: true },
    { name: tr('columnMd5'), col: 'md5', hideable: true, extraClass: 'col-md5' },
];

restoreViewSession();
updateHideUnknownLabel();
updateDynamicStyles();

// Column context menu
function showColumnMenu(event) {
    event.preventDefault();
    const menu = document.getElementById('col-context-menu');
    menu.innerHTML = '';
    columnDefs.forEach(def => {
        const item = document.createElement('div');
        item.className = 'col-context-item' + (!def.hideable ? ' disabled' : '');
        const checkbox = document.createElement('input');
        checkbox.type = 'checkbox';
        checkbox.checked = !hiddenColumns.has(def.col);
        checkbox.disabled = !def.hideable;
        const label = document.createElement('span');
        label.textContent = def.name;
        item.appendChild(checkbox);
        item.appendChild(label);
        if (def.hideable) {
            item.addEventListener('click', () => {
                const visible = hiddenColumns.has(def.col);
                checkbox.checked = visible;
                setColumnVisible(def.col, visible);
            });
        }
        menu.appendChild(item);
    });
    menu.style.left = Math.min(event.clientX, window.innerWidth - 180) + 'px';
    menu.style.top = Math.min(event.clientY, window.innerHeight - menu.offsetHeight - 8) + 'px';
    menu.classList.remove('d-none');
}

document.addEventListener('click', () => {
    document.getElementById('col-context-menu').classList.add('d-none');
});

// API actions
async function reloadAll() {
    setStatus(tr('statusReloading'));
    await fetch('/api/reload', { method: 'POST' });
    await Promise.all([loadCollections('stable'), loadCollections('lazer')]);
    setStatus(tr('statusReady'));
}

async function recompileScss() {
    setStatus(tr('statusRecompilingScss'));
    try {
        const response = await fetch('/api/recompile-scss', { method: 'POST' });
        const data = await response.json();
        if (data.status === 'ok') {
            setStatus(tr('statusScssRecompiled'));
        } else {
            setStatus(tr('statusScssError', { error: data.message || tr('unknownError') }));
        }
    } catch (error) {
        setStatus(tr('statusScssError', { error: error.message }));
    }
}

function sortList(listId, columnIndex) {
    const list = document.getElementById(listId);
    const state = listData[listId];
    if (!list || !state) return;

    const prevCol = state.sortCol;
    const prevDir = state.sortDir;
    let dir;
    if (prevCol !== columnIndex) dir = 1;
    else if (prevDir === 1) dir = -1;
    else if (prevDir === -1) dir = 0;
    else dir = 1;

    state.sortCol = columnIndex;
    state.sortDir = dir;

    list.querySelectorAll('.beatmap-header .beatmap-col').forEach((h, i) => {
        h.textContent = h.textContent.replace(/ [▲▼]$/, '');
        if (i === columnIndex && dir !== 0)
            h.textContent += dir === 1 ? ' ▲' : ' ▼';
    });

    const col = columnDefs[columnIndex]?.col;
    const isNumeric = col === 'stars' || col === 'id' || col === 'setid';

    state.sorted = dir === 0
        ? [...state.all]
        : [...state.all].sort((a, b) => dir * (isNumeric
            ? a._k[col] - b._k[col]
            : a._k[col].localeCompare(b._k[col])));
    refreshVisibleBeatmaps(state);

    startRender(list, listId);
}

function buildBeatmapRow(beatmap, mode, collectionName) {
    const notDownloaded = !beatmap.title;
    const row = cloneTemplate('tmpl-beatmap-row');
    row.dataset.md5 = beatmap.md5 || '';
    if (notDownloaded) {
        row.classList.add('text-muted', 'fst-italic');
        row.dataset.unknown = '1';
    }
    if (selectionFor(mode, collectionName).has(beatmap.md5)) {
        row.classList.add('beatmap-row-selected');
    }
    row.addEventListener('click', event => {
        if (event.target.closest('a')) return;
        event.preventDefault();
        const additive = event.ctrlKey || event.metaKey;
        if (event.shiftKey) selectBeatmapRange(row, mode, collectionName, beatmap.md5, additive);
        else selectBeatmapRow(row, mode, collectionName, beatmap.md5, additive);
    });
    row.addEventListener('contextmenu', e => showBeatmapMenu(e, mode, collectionName, beatmap.md5));

    const cells = {};
    row.querySelectorAll('[data-col]').forEach(el => { cells[el.dataset.col] = el; });

    columnDefs.forEach(def => {
        const cell = cells[def.col];
        if (!cell) return;
        if (def.extraClass) cell.classList.add(def.extraClass);

        switch (def.col) {
            case 'title': cell.textContent = notDownloaded ? tr('unknownBeatmapTitle') : beatmap.title; break;
            case 'artist': cell.textContent = beatmap.artist || ''; break;
            case 'difficulty': cell.textContent = beatmap.difficulty || ''; break;
            case 'mapper': cell.textContent = beatmap.mapper || ''; break;
            case 'stars': cell.textContent = formatStars(beatmap.stars); break;
            case 'id':
                if (beatmap.id > 0) {
                    const a = document.createElement('a');
                    a.href = 'https://osu.ppy.sh/b/' + beatmap.id;
                    a.target = '_blank'; a.textContent = beatmap.id;
                    cell.appendChild(a);
                }
                break;
            case 'setid':
                if (beatmap.setId > 0) {
                    const a = document.createElement('a');
                    a.href = 'https://osu.ppy.sh/beatmapsets/' + beatmap.setId;
                    a.target = '_blank'; a.textContent = beatmap.setId;
                    cell.appendChild(a);
                }
                break;
            case 'md5': cell.textContent = beatmap.md5 || ''; break;
        }
    });
    return row;
}

function buildBeatmapList(collection, listId, mode) {
    // Pre-compute lowercase sort keys so sort never touches the DOM
    const prepared = collection.beatmaps.map(b => ({
        ...b,
        _k: {
            title: (b.title || '').toLowerCase(),
            artist: (b.artist || '').toLowerCase(),
            difficulty: (b.difficulty || '').toLowerCase(),
            mapper: (b.mapper || '').toLowerCase(),
            stars: b.stars || 0,
            id: b.id || 0,
            setid: b.setId || 0,
            md5: (b.md5 || '').toLowerCase(),
        }
    }));
    const list = cloneTemplate('tmpl-beatmap-list');
    list.id = listId;

    const header = cloneTemplate('tmpl-beatmap-header');
    columnDefs.forEach((def, index) => {
        const col = cloneTemplate('tmpl-beatmap-header-col');
        col.textContent = def.name;
        col.dataset.col = def.col;
        if (def.extraClass) col.classList.add(def.extraClass);
        col.addEventListener('click', () => sortList(listId, index));
        header.appendChild(col);
    });
    list.appendChild(header);

    const body = document.createElement('div');
    body.className = 'beatmap-virtual-body';
    const rowsWindow = document.createElement('div');
    rowsWindow.className = 'beatmap-virtual-window';
    body.appendChild(rowsWindow);

    list.appendChild(body);

    listData[listId] = {
        all: prepared,
        sorted: prepared,
        visible: prepared,
        sortCol: -1,
        sortDir: 0,
        mode,
        collectionName: collection.name,
        list,
        scrollEl: null,
        header,
        body,
        rowsWindow,
        rowStep: DEFAULT_ROW_STEP,
        renderScheduled: false,
    };

    requestAnimationFrame(() => {
        const state = listData[listId];
        if (!state || !list.isConnected) return;
        state.scrollEl = list.parentElement;
        state.scrollEl.addEventListener('scroll', () => scheduleVirtualRender(listId), { passive: true });
        startRender(list, listId);
    });

    return list;
}

function renderBeatmaps(collection, mode) {
    const fragment = document.createDocumentFragment();
    if (!collection.beatmaps || collection.beatmaps.length === 0) {
        const p = document.createElement('p');
        p.className = 'text-muted p-2 m-0';
        p.innerHTML = tr('noBeatmaps', { name: escapeHtmlKeepLeadingSpaces(collection.name) });
        fragment.appendChild(p);
        return fragment;
    }
    const listId = 'lst-' + (++detailSequence);

    const header = cloneTemplate('tmpl-collection-header');
    header.querySelector('[data-slot="name"]').innerHTML = escapeHtmlKeepLeadingSpaces(collection.name);
    header.querySelector('[data-slot="count"]').textContent = tr('mapCount', { count: collection.beatmaps.length });
    fragment.appendChild(header);
    fragment.appendChild(buildBeatmapList(collection, listId, mode));

    return fragment;
}

async function loadCollections(mode) {
    const listElement = document.getElementById(mode + '-list');
    try {
        const response = await fetch('/api/' + mode + '/collections');
        if (!response.ok) throw new Error(await response.text());
        const collections = await response.json();

        const label = mode[0].toUpperCase() + mode.slice(1);
        collectionCounts[label] = collections.length;
        updateStatusCounts();

        if (collections.length === 0) {
            const div = document.createElement('div');
            div.className = 'list-group-item text-muted py-1 border-0 fst-italic';
            div.textContent = tr('noCollections');
            listElement.replaceChildren(div);
            return;
        }

        const fragment = document.createDocumentFragment();
        collections.forEach(collection => {
            const item = cloneTemplate('tmpl-collection-item');
            item.dataset.name = collection.name || '';
            item.querySelector('[data-slot="name"]').innerHTML =
                escapeHtmlKeepLeadingSpaces(collection.name) || '<em class="text-muted">' + tr('unnamedCollection') + '</em>';
            item.querySelector('[data-slot="count"]').textContent = collection.count;
            item.addEventListener('click', () => selectCollection(mode, item));
            item.addEventListener('contextmenu', e => showCollectionMenu(e, mode, collection.name || ''));
            fragment.appendChild(item);
        });
        listElement.replaceChildren(fragment);

    } catch (error) {
        const div = document.createElement('div');
        div.className = 'list-group-item text-danger py-1 border-0';
        div.textContent = error.message;
        listElement.replaceChildren(div);
        setStatus(tr('genericError', { error: error.message }));
    }
}

async function selectCollection(mode, element) {
    document.querySelectorAll('#' + mode + '-list .list-group-item')
        .forEach(item => item.classList.remove('active'));
    element.classList.add('active');

    const name = element.dataset.name;
    const detailPanel = document.getElementById(mode + '-detail');
    const loading = document.createElement('p');
    loading.className = 'text-muted p-2 m-0';
    loading.textContent = tr('paneLoading');
    detailPanel.replaceChildren(loading);
    document.getElementById('status-text').innerHTML = tr('loadingCollection', { name: escapeHtmlKeepLeadingSpaces(name) });

    try {
        const response = await fetch('/api/' + mode + '/collections?name=' + encodeURIComponent(name));
        if (!response.ok) throw new Error(await response.text());
        const collection = await response.json();
        detailPanel.replaceChildren(renderBeatmaps(collection, mode));
        document.getElementById('status-text').innerHTML =
            tr('collectionStatus', { name: escapeHtmlKeepLeadingSpaces(name), count: collection.beatmaps ? collection.beatmaps.length : 0 });
    } catch (error) {
        const div = document.createElement('div');
        div.className = 'alert alert-danger m-2 py-1 rounded-0';
        div.textContent = error.message;
        detailPanel.replaceChildren(div);
        setStatus(tr('genericError', { error: error.message }));
    }
}

loadCollections('stable');
loadCollections('lazer');


// Collection context menu
function showCollectionMenu(event, mode, name) {
    event.preventDefault();
    event.stopPropagation();
    const otherMode = mode === 'stable' ? 'lazer' : 'stable';
    const menu = document.getElementById('coll-context-menu');
    menu.innerHTML = '';

    const items = [
        { label: tr('menuRename'), action: () => renameCollection(mode, name) },
        { label: tr('menuExportOsdb'), action: () => exportCollections(mode, [name]) },
        { label: tr('menuCopyTo', { mode: otherMode }), action: () => copyCollectionTo(mode, otherMode, name) },
        beatmapClipboard ? { label: tr('menuPasteBeatmaps', { count: beatmapClipboard.hashes.length }), action: () => pasteBeatmaps(mode, name) } : null,
        'separator',
        { label: tr('menuDelete'), action: () => deleteCollection(mode, name), danger: true },
    ];

    items.forEach(item => {
        if (!item) return;
        if (item === 'separator') { const hr = document.createElement('hr'); hr.className = 'dropdown-divider my-0'; menu.appendChild(hr); return; }
        const el = document.createElement('div');
        el.className = 'col-context-item' + (item.danger ? ' text-danger' : '');
        el.textContent = item.label;
        el.addEventListener('click', () => { menu.classList.add('d-none'); item.action(); });
        menu.appendChild(el);
    });

    menu.style.left = Math.min(event.clientX, window.innerWidth - 180) + 'px';
    menu.style.top  = Math.min(event.clientY, window.innerHeight - menu.scrollHeight - 8) + 'px';
    menu.classList.remove('d-none');
}

document.addEventListener('click', () => {
    document.getElementById('coll-context-menu').classList.add('d-none');
    document.getElementById('beatmap-context-menu').classList.add('d-none');
});

// Beatmap row context menu
function showBeatmapMenu(event, mode, collectionName, md5) {
    event.preventDefault();
    event.stopPropagation();
    const otherMode = mode === 'stable' ? 'lazer' : 'stable';
    const hashes = selectedHashesFor(mode, collectionName, md5);
    const menu = document.getElementById('beatmap-context-menu');
    menu.innerHTML = '';

    const items = [
        { label: tr('menuCopyBeatmaps', { count: hashes.length }), action: () => copyBeatmaps(mode, collectionName, hashes) },
        { label: tr('menuRemoveFromCollection'), action: async () => {
            await apiFetch('/api/' + mode + '/collections/remove-beatmaps', { collection: collectionName, hashes });
            await selectCollectionByName(mode, collectionName);
        }, danger: true },
        { label: tr('menuCopyTo', { mode: otherMode }), action: async () => {
            const res = await apiFetch('/api/collections/copy', { from: mode, to: otherMode, collection: collectionName, hashes });
            if (res.status === 'ok') setStatus(tr('copiedBeatmapsTo', { count: hashes.length, mode: otherMode }));
        }},
    ];

    items.forEach(item => {
        const el = document.createElement('div');
        el.className = 'col-context-item' + (item.danger ? ' text-danger' : '');
        el.textContent = item.label;
        el.addEventListener('click', () => { menu.classList.add('d-none'); item.action(); });
        menu.appendChild(el);
    });

    menu.style.left = Math.min(event.clientX, window.innerWidth - 180) + 'px';
    menu.style.top  = Math.min(event.clientY, window.innerHeight - menu.scrollHeight - 8) + 'px';
    menu.classList.remove('d-none');
}

// API helpers
async function apiFetch(url, body) {
    const res = await fetch(url, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(body),
    });
    if (!res.ok) throw new Error(await res.text());
    return res.json();
}

// Collection CRUD
async function createCollection(mode) {
    const name = prompt(tr('promptCollectionName'));
    if (!name) return;
    try {
        await apiFetch('/api/' + mode + '/collections/create', { name });
        await loadCollections(mode);
        setStatus(tr('statusCreatedCollection', { name }));
    } catch (e) { setStatus(tr('genericError', { error: e.message })); }
}

async function deleteCollection(mode, name) {
    if (!confirm(tr('confirmDeleteCollection', { name }))) return;
    try {
        await apiFetch('/api/' + mode + '/collections/delete', { name });
        await loadCollections(mode);
        document.getElementById(mode + '-detail').innerHTML = '';
        setStatus(tr('statusDeletedCollection', { name }));
    } catch (e) { setStatus(tr('genericError', { error: e.message })); }
}

async function renameCollection(mode, name) {
    const newName = prompt(tr('promptNewName'), name);
    if (!newName || newName === name) return;
    try {
        await apiFetch('/api/' + mode + '/collections/rename', { name, newName });
        await loadCollections(mode);
        setStatus(tr('statusRenamedCollection', { name: newName }));
    } catch (e) { setStatus(tr('genericError', { error: e.message })); }
}

async function selectCollectionByName(mode, name) {
    const item = [...document.querySelectorAll('#' + mode + '-list .list-group-item')]
        .find(el => el.dataset.name === name);
    if (item) await selectCollection(mode, item);
}

async function copyCollectionTo(fromMode, toMode, name) {
    try {
        const res = await apiFetch('/api/collections/copy', { from: fromMode, to: toMode, collection: name });
        setStatus(tr('copiedBeatmapsTo', { count: res.copied, mode: toMode }));
    } catch (e) { setStatus(tr('genericError', { error: e.message })); }
}

// Save
async function saveCollections(mode) {
    setStatus(tr('statusSaving', { mode }));
    try {
        await apiFetch('/api/' + mode + '/save', {});
        setStatus(tr('statusSaved', { mode }));
    } catch (e) { setStatus(tr('saveError', { error: e.message })); }
}

async function backupCollections(mode) {
    setStatus(tr('statusBackingUp', { mode }));
    try {
        const result = await apiFetch('/api/' + mode + '/backup', {});
        setStatus(tr('statusBackupCreated', { mode, path: result.path }));
    } catch (e) { setStatus(tr('backupError', { error: e.message })); }
}

// Import osdb
function importCollections(mode) {
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = '.osdb';
    input.onchange = async () => {
        const file = input.files[0];
        if (!file) return;
        setStatus(tr('statusImporting', { file: file.name }));
        try {
            const buffer = await file.arrayBuffer();
            const res = await fetch('/api/' + mode + '/import', {
                method: 'POST',
                headers: { 'Content-Type': 'application/octet-stream' },
                body: buffer,
            });
            if (!res.ok) throw new Error(await res.text());
            const data = await res.json();
            await loadCollections(mode);
            setStatus(tr('statusImportedCollections', { count: data.imported, mode }));
        } catch (e) { setStatus(tr('importError', { error: e.message })); }
    };
    input.click();
}

// Export osdb
async function exportCollections(mode, names) {
    try {
        const res = await fetch('/api/' + mode + '/export', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ collections: names }),
        });
        if (!res.ok) throw new Error(await res.text());
        const blob = await res.blob();
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = (names.length === 1 ? names[0] : 'collections') + '.osdb';
        a.click();
        URL.revokeObjectURL(url);
    } catch (e) { setStatus(tr('exportError', { error: e.message })); }
}

