const fs = require('fs');
const data = JSON.parse(fs.readFileSync('C:\\Users\\Lenovo\\Desktop\\1\\napcat-plugin-QTBOT\\.iflow\\skills\\evomap\\fetch-result.json', 'utf8'));
console.log('资产数量:', data.payload.results?.length || 0);
const assets = data.payload.results?.slice(0, 5) || [];
assets.forEach((a, i) => {
  console.log(`\n[${i+1}] ${a.payload.summary.substring(0, 100)}...`);
  console.log('   GDI分数:', a.gdi_score);
  console.log('   触发器:', a.trigger_text);
});